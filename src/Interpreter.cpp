#include "Interpreter.hpp"
#include "ReturnException.hpp"
#include "RuntimeError.hpp"
#include "TokenType.hpp"
#include <iostream>
#include <string>

// Main entry point
void Interpreter::interpret(
    const std::vector<std::shared_ptr<Stmt>> &statements) {
  // Error handling main vich hovega (Main banda fix karega)
  for (const auto &stmt : statements) {
    if (stmt)
      execute(stmt);
  }
}

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

    for (const auto &stmt : statements) {
      execute(stmt);
    }
  } catch (...) {
    this->environment = previous; // Wapis purane ghar
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
  return environment->get(expr.name);
}

std::any Interpreter::visitAssignExpr(Assign &expr) {
  std::any value = evaluate(expr.value);
  environment->assign(expr.name, value);
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

  // Call logic will be here (Classes/Functions not fully implemented yet)
  // Codecrafters usually puts functions in later stages.
  // We would need LoxCallable interface.
  // Throw error for now if not callable
  throw RuntimeError(expr.paren, "Can only call functions and classes.");
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
  // Defines function in environment.
  // Not implementing LoxFunction yet.
  return std::any();
}

std::any Interpreter::visitReturnStmt(Return &stmt) {
  std::any value = std::any();
  if (stmt.value != nullptr)
    value = evaluate(stmt.value);

  throw ReturnException(value);
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

  return "unknown"; // shouldn't happen
}
