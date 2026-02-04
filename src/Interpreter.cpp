#include "Interpreter.hpp"
#include "ReturnException.hpp"
#include "RuntimeError.hpp"
#include "TokenType.hpp"
#include <iostream>
#include <string>

#include "LoxCallable.hpp"
#include "LoxClass.hpp"
#include "LoxFunction.hpp"
#include "LoxInstance.hpp"

#include <ctime>

// Native clock function
// (Native clock function: Time is ticking!)
class Clock : public LoxCallable {
public:
  int arity() override { return 0; }

  std::any call(Interpreter &interpreter,
                std::vector<std::any> arguments) override {
    return (double)clock() / CLOCKS_PER_SEC;
  }

  std::string toString() override { return "<native fn>"; }
};

Interpreter::Interpreter() {
  globals = std::make_shared<Environment>();
  environment = globals;

  // Define native functions
  // (Defining native functions)
  globals->define("clock",
                  std::shared_ptr<LoxCallable>(std::make_shared<Clock>()));
}

// Main entry point
void Interpreter::interpret(
    const std::vector<std::shared_ptr<Stmt>> &statements) {
  // Error handling main vich hovega (Main banda fix karega)
  // (Error handling will be in main, the main boss will fix it)
  for (const auto &stmt : statements) {
    if (stmt)
      execute(stmt);
  }
}

void Interpreter::resolve(Expr *expr, int depth) { locals[expr] = depth; }

std::any Interpreter::evaluate(std::shared_ptr<Expr> expr) {
  return expr->accept(*this);
}

void Interpreter::execute(std::shared_ptr<Stmt> stmt) { stmt->accept(*this); }

void Interpreter::executeBlock(
    const std::vector<std::shared_ptr<Stmt>> &statements,
    std::shared_ptr<Environment> environment) {
  std::shared_ptr<Environment> previous = this->environment;

  try {
    this->environment = environment; // Scope switch (Ghar badlo)
                                     // (Scope switch: Change houses)

    for (const auto &stmt : statements) {
      execute(stmt);
    }
  } catch (...) {
    this->environment = previous; // Wapis purane ghar
                                  // (Back to the old house)
    throw;
  }
  this->environment = previous;
}

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
    checkNumberOperand(expr.op, right);
    return -std::any_cast<double>(right);
  default:
    return std::any(); // Should not happen
  }
}

std::any Interpreter::visitBinaryExpr(Binary &expr) {
  std::any left = evaluate(expr.left);
  std::any right = evaluate(expr.right);

  switch (expr.op.type) {
  case TokenType::GREATER:
    checkNumberOperands(expr.op, left, right);
    return std::any_cast<double>(left) > std::any_cast<double>(right);
  case TokenType::GREATER_EQUAL:
    checkNumberOperands(expr.op, left, right);
    return std::any_cast<double>(left) >= std::any_cast<double>(right);
  case TokenType::LESS:
    checkNumberOperands(expr.op, left, right);
    return std::any_cast<double>(left) < std::any_cast<double>(right);
  case TokenType::LESS_EQUAL:
    checkNumberOperands(expr.op, left, right);
    return std::any_cast<double>(left) <= std::any_cast<double>(right);
  case TokenType::BANG_EQUAL:
    return !isEqual(left, right);
  case TokenType::EQUAL_EQUAL:
    return isEqual(left, right);
  case TokenType::MINUS:
    checkNumberOperands(expr.op, left, right);
    return std::any_cast<double>(left) - std::any_cast<double>(right);
  case TokenType::PLUS:
    if (left.type() == typeid(double) && right.type() == typeid(double)) {
      return std::any_cast<double>(left) + std::any_cast<double>(right);
    }
    if (left.type() == typeid(std::string) &&
        right.type() == typeid(std::string)) {
      return std::any_cast<std::string>(left) +
             std::any_cast<std::string>(right);
    }
    // C++ specific: if one is string and other is not, maybe concat? check
    // specs. Usually Lox requires both string or both number.
    throw RuntimeError(expr.op, "Operands must be two numbers or two strings.");
  case TokenType::SLASH:
    checkNumberOperands(expr.op, left, right);
    return std::any_cast<double>(left) / std::any_cast<double>(right);
  case TokenType::STAR:
    checkNumberOperands(expr.op, left, right);
    return std::any_cast<double>(left) * std::any_cast<double>(right);
  default:
    return std::any();
  }
}

std::any Interpreter::visitVariableExpr(Variable &expr) {
  // Check if local
  auto it = locals.find(&expr);
  if (it != locals.end()) {
    return environment->getAt(it->second, expr.name.lexeme);
  } else {
    return globals->get(expr.name);
  }
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
  std::any left = evaluate(expr.left);

  if (expr.op.type == TokenType::OR) {
    if (isTruthy(left))
      return left;
  } else {
    if (!isTruthy(left))
      return left;
  }

  return evaluate(expr.right);
}

std::any Interpreter::visitCallExpr(Call &expr) {
  std::any callee = evaluate(expr.callee);

  std::vector<std::any> arguments;
  for (const auto &arg : expr.arguments) {
    arguments.push_back(evaluate(arg));
  }

  std::shared_ptr<LoxCallable> function;
  if (callee.type() == typeid(std::shared_ptr<LoxFunction>)) {
    function = std::any_cast<std::shared_ptr<LoxFunction>>(callee);
  } else if (callee.type() == typeid(std::shared_ptr<LoxClass>)) {
    function = std::any_cast<std::shared_ptr<LoxClass>>(callee);
  } else if (callee.type() == typeid(std::shared_ptr<LoxCallable>)) {
    function = std::any_cast<std::shared_ptr<LoxCallable>>(callee);
  } else {
    throw RuntimeError(expr.paren, "Can only call functions and classes.");
  }

  if (arguments.size() != function->arity()) {
    throw RuntimeError(expr.paren, "Expected " +
                                       std::to_string(function->arity()) +
                                       " arguments but got " +
                                       std::to_string(arguments.size()) + ".");
  }

  return function->call(*this, arguments);
}

// --- Stmt Visitor ---

std::any Interpreter::visitExpressionStmt(Expression &stmt) {
  evaluate(stmt.expression);
  return std::any();
}

std::any Interpreter::visitPrintStmt(Print &stmt) {
  std::any value = evaluate(stmt.expression);
  std::cout << stringify(value) << std::endl;
  return std::any();
}

std::any Interpreter::visitVarStmt(Var &stmt) {
  std::any value = std::any();
  if (stmt.initializer != nullptr) {
    value = evaluate(stmt.initializer);
  }
  environment->define(stmt.name.lexeme, value);
  return std::any();
}

std::any Interpreter::visitBlockStmt(Block &stmt) {
  executeBlock(stmt.statements, std::make_shared<Environment>(environment));
  return std::any();
}

std::any Interpreter::visitIfStmt(If &stmt) {
  if (isTruthy(evaluate(stmt.condition))) {
    execute(stmt.thenBranch);
  } else if (stmt.elseBranch != nullptr) {
    execute(stmt.elseBranch);
  }
  return std::any();
}

std::any Interpreter::visitWhileStmt(While &stmt) {
  while (isTruthy(evaluate(stmt.condition))) {
    execute(stmt.body);
  }
  return std::any();
}

std::any Interpreter::visitFunctionStmt(Function &stmt) {
  // Capture environment for closure
  std::shared_ptr<LoxFunction> function =
      std::make_shared<LoxFunction>(stmt, environment);
  environment->define(stmt.name.lexeme, function);
  return std::any();
}

std::any Interpreter::visitReturnStmt(Return &stmt) {
  std::any value = std::any();
  if (stmt.value != nullptr)
    value = evaluate(stmt.value);

  throw ReturnException(value);
}

std::any Interpreter::visitClassStmt(Class &stmt) {
  // Superclass evaluate karo agar hai (Evaluate superclass if present)
  std::shared_ptr<LoxClass> superclass = nullptr;
  if (stmt.superclass != nullptr) {
    std::any superValue =
        evaluate(std::make_shared<Variable>(stmt.superclass->name));
    if (superValue.type() != typeid(std::shared_ptr<LoxClass>)) {
      throw RuntimeError(stmt.superclass->name, "Superclass must be a class.");
    }
    superclass = std::any_cast<std::shared_ptr<LoxClass>>(superValue);
  }

  // Class da naam define karo (Define the class name)
  environment->define(stmt.name.lexeme, std::any());

  // Agar superclass hai ta nava environment banao jis vich "super" hai
  // (If superclass exists, create new environment with "super")
  if (stmt.superclass != nullptr) {
    environment = std::make_shared<Environment>(environment);
    environment->define("super", superclass);
  }

  // Methods nu bind karo (Bind the methods)
  std::map<std::string, std::shared_ptr<LoxFunction>> methods;
  for (const auto &method : stmt.methods) {
    std::shared_ptr<LoxFunction> function =
        std::make_shared<LoxFunction>(*method, environment);
    methods[method->name.lexeme] = function;
  }

  // Class banao (Create the class)
  std::shared_ptr<LoxClass> klass =
      std::make_shared<LoxClass>(stmt.name.lexeme, superclass, methods);

  // Agar superclass hai ta environment wapas lao
  // (If superclass existed, restore environment)
  if (superclass != nullptr) {
    environment = environment->enclosing;
  }

  environment->assign(stmt.name, klass);
  return std::any();
}

std::any Interpreter::visitGetExpr(Get &expr) {
  std::any object = evaluate(expr.object);
  if (object.type() == typeid(std::shared_ptr<LoxInstance>)) {
    return std::any_cast<std::shared_ptr<LoxInstance>>(object)->get(expr.name);
  }
  throw RuntimeError(expr.name, "Only instances have properties.");
}

std::any Interpreter::visitSetExpr(Set &expr) {
  std::any object = evaluate(expr.object);

  if (object.type() != typeid(std::shared_ptr<LoxInstance>)) {
    throw RuntimeError(expr.name, "Only instances have fields.");
  }

  std::any value = evaluate(expr.value);
  std::any_cast<std::shared_ptr<LoxInstance>>(object)->set(expr.name, value);
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
  std::shared_ptr<LoxClass> superclass =
      std::any_cast<std::shared_ptr<LoxClass>>(superValue);

  // "this" labho jo ek level upar hai (Get "this" which is one level above)
  std::any thisValue = environment->getAt(distance - 1, "this");
  std::shared_ptr<LoxInstance> object =
      std::any_cast<std::shared_ptr<LoxInstance>>(thisValue);

  // Superclass vich method labho (Find the method in superclass)
  std::shared_ptr<LoxFunction> method =
      superclass->findMethod(expr.method.lexeme);

  if (method == nullptr) {
    throw RuntimeError(expr.method,
                       "Undefined property '" + expr.method.lexeme + "'.");
  }

  // Method nu current object naal bind karo (Bind the method to current object)
  return method->bind(object);
}

// --- Helpers ---

bool Interpreter::isTruthy(std::any object) {
  if (!object.has_value())
    return false; // nil
  if (object.type() == typeid(bool))
    return std::any_cast<bool>(object);
  return true; // everything else is true
}

bool Interpreter::isEqual(std::any a, std::any b) {
  if (!a.has_value() && !b.has_value())
    return true;
  if (!a.has_value())
    return false;

  if (a.type() == typeid(bool) && b.type() == typeid(bool))
    return std::any_cast<bool>(a) == std::any_cast<bool>(b);
  if (a.type() == typeid(double) && b.type() == typeid(double))
    return std::any_cast<double>(a) == std::any_cast<double>(b);
  if (a.type() == typeid(std::string) && b.type() == typeid(std::string))
    return std::any_cast<std::string>(a) == std::any_cast<std::string>(b);

  return false;
}

void Interpreter::checkNumberOperand(Token operatorToken, std::any operand) {
  if (operand.type() == typeid(double))
    return;
  throw RuntimeError(operatorToken, "Operand must be a number.");
}

void Interpreter::checkNumberOperands(Token operatorToken, std::any left,
                                      std::any right) {
  if (left.type() == typeid(double) && right.type() == typeid(double))
    return;
  throw RuntimeError(operatorToken, "Operands must be numbers.");
}

std::string Interpreter::stringify(std::any object) {
  if (!object.has_value())
    return "nil";

  if (object.type() == typeid(double)) {
    std::string text = std::to_string(std::any_cast<double>(object));
    // Remove trailing zeros
    if (text.find('.') != std::string::npos) {
      text = text.substr(0, text.find_last_not_of('0') + 1);
      if (text.back() == '.') {
        text = text.substr(0, text.length() - 1);
      }
    }
    return text;
  }

  if (object.type() == typeid(std::string))
    return std::any_cast<std::string>(object);
  if (object.type() == typeid(bool))
    return std::any_cast<bool>(object) ? "true" : "false";
  if (object.type() == typeid(std::shared_ptr<LoxClass>))
    return std::any_cast<std::shared_ptr<LoxClass>>(object)->toString();
  if (object.type() == typeid(std::shared_ptr<LoxInstance>))
    return std::any_cast<std::shared_ptr<LoxInstance>>(object)->toString();
  if (object.type() == typeid(std::shared_ptr<LoxFunction>))
    return std::any_cast<std::shared_ptr<LoxFunction>>(object)->toString();

  return "unknown"; // shouldn't happen
}
