#include "Interpreter.hpp"

#include "LoxClass.hpp"
#include "LoxFunction.hpp"
#include "LoxInstance.hpp"
#include "ReturnException.hpp"
#include "RuntimeError.hpp"
#include "TokenType.hpp"
#include "value.hpp"

#include <any>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace lox {

// The expression half of the tree walk.
//
// Every method here is a pure function of (expression, environment): evaluate
// a node, hand back a value. Nothing in this file changes the current scope -
// that is executeBlock's job, and it lives in Interpreter.cpp.

// --- Expr Visitor ---

std::any Interpreter::visitLiteralExpr(Literal &expr) { return expr.value; }

std::any Interpreter::visitGroupingExpr(Grouping &expr) {
  return evaluate(expr.expression);
}

std::any Interpreter::visitUnaryExpr(Unary &expr) {
  std::any right = evaluate(expr.right);

  switch (expr.op.type) {
  case TokenType::BANG:
    return !isTruthy(right);
  case TokenType::MINUS:
    requireNumber(expr.op, right);
    return -asNumber(right);
  default:
    return std::any(); // Should not happen
  }
}

std::any Interpreter::visitBinaryExpr(Binary &expr) {
  std::any left = evaluate(expr.left);
  std::any right = evaluate(expr.right);

  switch (expr.op.type) {
  case TokenType::GREATER:
    requireNumbers(expr.op, left, right);
    return asNumber(left) > asNumber(right);
  case TokenType::GREATER_EQUAL:
    requireNumbers(expr.op, left, right);
    return asNumber(left) >= asNumber(right);
  case TokenType::LESS:
    requireNumbers(expr.op, left, right);
    return asNumber(left) < asNumber(right);
  case TokenType::LESS_EQUAL:
    requireNumbers(expr.op, left, right);
    return asNumber(left) <= asNumber(right);
  case TokenType::BANG_EQUAL:
    return !isEqual(left, right);
  case TokenType::EQUAL_EQUAL:
    return isEqual(left, right);
  case TokenType::MINUS:
    requireNumbers(expr.op, left, right);
    return asNumber(left) - asNumber(right);
  case TokenType::PLUS:
    if (isNumber(left) && isNumber(right)) {
      return asNumber(left) + asNumber(right);
    }
    if (isString(left) && isString(right)) {
      return asString(left) + asString(right);
    }
    // C++ specific: if one is string and other is not, maybe concat? check
    // specs. Usually Lox requires both string or both number.
    throw RuntimeError(expr.op, "Operands must be two numbers or two strings.");
  case TokenType::SLASH:
    requireNumbers(expr.op, left, right);
    return asNumber(left) / asNumber(right);
  case TokenType::STAR:
    requireNumbers(expr.op, left, right);
    return asNumber(left) * asNumber(right);
  default:
    return std::any();
  }
}

std::any Interpreter::visitVariableExpr(Variable &expr) {
  // The Resolver recorded a depth for every local; absence means global.
  auto it = locals.find(&expr);
  if (it != locals.end()) {
    return environment->getAt(it->second, expr.name.lexeme);
  }
  return globals->get(expr.name);
}

std::any Interpreter::visitAssignExpr(Assign &expr) {
  std::any value = evaluate(expr.value);

  auto it = locals.find(&expr);
  if (it != locals.end()) {
    environment->assignAt(it->second, expr.name, value);
  } else {
    globals->assign(expr.name, value);
  }

  return value;
}

std::any Interpreter::visitLogicalExpr(Logical &expr) {
  // and/or short-circuit and return the operand value, not a bool.
  std::any left = evaluate(expr.left);

  if (expr.op.type == TokenType::OR) {
    if (isTruthy(left)) {
      return left;
    }
  } else {
    if (!isTruthy(left)) {
      return left;
    }
  }

  return evaluate(expr.right);
}

std::any Interpreter::visitCallExpr(Call &expr) {
  std::any callee = evaluate(expr.callee);

  std::vector<std::any> arguments;
  arguments.reserve(expr.arguments.size());
  for (const auto &arg : expr.arguments) {
    arguments.push_back(evaluate(arg));
  }

  // A LoxFunction and a LoxClass are stored as their own types so they can be
  // told apart; a native is stored as the base LoxCallable.
  std::shared_ptr<LoxCallable> function;
  if (callee.type() == typeid(std::shared_ptr<LoxFunction>)) {
    function = asFunction(callee);
  } else if (callee.type() == typeid(std::shared_ptr<LoxClass>)) {
    function = asClass(callee);
  } else if (callee.type() == typeid(std::shared_ptr<LoxCallable>)) {
    function = std::any_cast<std::shared_ptr<LoxCallable>>(callee);
  } else {
    throw RuntimeError(expr.paren, "Can only call functions and classes.");
  }

  if (arguments.size() != static_cast<size_t>(function->arity())) {
    throw RuntimeError(expr.paren,
                       "Expected " + std::to_string(function->arity()) +
                           " arguments but got " +
                           std::to_string(arguments.size()) + ".");
  }

  return function->call(*this, std::move(arguments));
}

std::any Interpreter::visitGetExpr(Get &expr) {
  std::any object = evaluate(expr.object);
  if (object.type() == typeid(std::shared_ptr<LoxInstance>)) {
    return asInstance(object)->get(expr.name);
  }
  throw RuntimeError(expr.name, "Only instances have properties.");
}

std::any Interpreter::visitSetExpr(Set &expr) {
  std::any object = evaluate(expr.object);

  if (object.type() != typeid(std::shared_ptr<LoxInstance>)) {
    throw RuntimeError(expr.name, "Only instances have fields.");
  }

  // Lox evaluates the object before the value, so `a[f()] = g()` runs f first.
  std::any value = evaluate(expr.value);
  asInstance(object)->set(expr.name, value);
  return value;
}

std::any Interpreter::visitThisExpr(This &expr) {
  auto it = locals.find(&expr);
  if (it != locals.end()) {
    return environment->getAt(it->second, "this");
  }
  return globals->get(expr.keyword);
}

std::any Interpreter::visitSuperExpr(Super &expr) {
  // Super di depth labho (Find the depth of super)
  auto it = locals.find(&expr);
  int distance = it->second;

  // Superclass labho (Get the superclass)
  std::any superValue = environment->getAt(distance, "super");
  std::shared_ptr<LoxClass> superclass = asClass(superValue);

  // "this" labho jo ek level upar hai (Get "this" which is one level above)
  std::any thisValue = environment->getAt(distance - 1, "this");
  std::shared_ptr<LoxInstance> object = asInstance(thisValue);

  // Superclass vich method labho (Find the method in superclass)
  std::shared_ptr<LoxFunction> method = superclass->findMethod(expr.method.lexeme);

  if (method == nullptr) {
    throw RuntimeError(expr.method,
                       "Undefined property '" + expr.method.lexeme + "'.");
  }

  // Method nu current object naal bind karo (Bind the method to current object)
  return method->bind(std::move(object));
}

} // namespace lox
