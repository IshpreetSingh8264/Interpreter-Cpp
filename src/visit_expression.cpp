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
    // The parser only ever builds a Unary for `!` and `-`. Reaching this means
    // the grammar and this switch disagree, which is an interpreter bug, not a
    // program error. Returning nil instead would let it reach `print` and
    // surface as the value "nil".
    throw RuntimeError(expr.op, "Invalid unary operator.");
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
    // Same reasoning as visitUnaryExpr: the grammar cannot produce this, so a
    // mismatch is an interpreter bug and must not read as nil.
    throw RuntimeError(expr.op, "Invalid binary operator.");
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

// The Resolver binds `this` at a known depth, or reports "Can't use 'this'
// outside of a class" and stops the program before it runs. Reaching the
// fallback means the AST reached the tree walker without passing the Resolver,
// and there is no global named `this` to fall back to anyway.
std::any Interpreter::visitThisExpr(This &expr) {
  auto it = locals.find(&expr);
  if (it == locals.end()) {
    throw RuntimeError(expr.keyword, "Can't use 'this' outside of a class.");
  }
  return environment->getSlotOrFail(it->second, expr.keyword, "this");
}

// The Resolver binds `super` only inside a subclass, and reports
// "Can't use 'super' outside of a class" or "... in a class with no
// superclass" before the program runs. locals.find() therefore always has an
// entry here; the guard is what turns a missed entry into that error instead of
// dereferencing the end iterator.
std::any Interpreter::visitSuperExpr(Super &expr) {
  // Super di depth labho (Find the depth of super)
  auto it = locals.find(&expr);
  if (it == locals.end()) {
    throw RuntimeError(expr.keyword, "Can't use 'super' outside of a class.");
  }
  int distance = it->second;

  // Superclass labho (Get the superclass)
  std::shared_ptr<LoxClass> superclass =
      asClass(environment->getSlotOrFail(distance, expr.keyword, "super"));

  // "this" labho jo ek level upar hai (Get "this" which is one level above)
  std::shared_ptr<LoxInstance> object = asInstance(
      environment->getSlotOrFail(distance - 1, expr.keyword, "this"));

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
