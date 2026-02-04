#include "Resolver.hpp"
#include <iostream>

void Resolver::resolve(const std::vector<std::shared_ptr<Stmt>> &statements) {
  for (const auto &stmt : statements) {
    resolve(stmt);
  }
}

void Resolver::resolve(std::shared_ptr<Stmt> stmt) {
  if (stmt != nullptr) {
    stmt->accept(*this);
  }
}

void Resolver::resolve(std::shared_ptr<Expr> expr) {
  if (expr != nullptr) {
    expr->accept(*this);
  }
}

void Resolver::beginScope() { scopes.push_back(std::map<std::string, bool>()); }

void Resolver::endScope() { scopes.pop_back(); }

void Resolver::declare(Token name) {
  if (scopes.empty())
    return;

  std::map<std::string, bool> &scope = scopes.back();
  if (scope.count(name.lexeme)) {
    std::cerr << "Error: Variable " << name.lexeme
              << " already declared in this scope." << std::endl;
  }

  scope[name.lexeme] = false;
}

void Resolver::define(Token name) {
  if (scopes.empty())
    return;
  scopes.back()[name.lexeme] = true;
}

void Resolver::resolveLocal(Expr *expr, Token name) {
  for (int i = scopes.size() - 1; i >= 0; i--) {
    if (scopes[i].count(name.lexeme)) {
      interpreter.resolve(expr, scopes.size() - 1 - i);
      return;
    }
  }
}

// Visitor implementations

std::any Resolver::visitBlockStmt(Block &stmt) {
  beginScope();
  resolve(stmt.statements);
  endScope();
  return std::any();
}

std::any Resolver::visitVarStmt(Var &stmt) {
  declare(stmt.name);
  if (stmt.initializer != nullptr) {
    resolve(stmt.initializer);
  }
  define(stmt.name);
  return std::any();
}

std::any Resolver::visitVariableExpr(Variable &expr) {
  if (!scopes.empty()) {
    auto &scope = scopes.back();
    if (scope.count(expr.name.lexeme) && scope[expr.name.lexeme] == false) {
      std::cerr << "Error: Can't read local variable in its own initializer."
                << std::endl;
    }
  }

  resolveLocal(&expr, expr.name);
  return std::any();
}

std::any Resolver::visitAssignExpr(Assign &expr) {
  resolve(expr.value);
  resolveLocal(&expr, expr.name);
  return std::any();
}

std::any Resolver::visitFunctionStmt(Function &stmt) {
  declare(stmt.name);
  define(stmt.name);

  beginScope();
  for (const auto &param : stmt.params) {
    declare(param);
    define(param);
  }
  resolve(stmt.body);
  endScope();
  return std::any();
}

// Traversal for other nodes

std::any Resolver::visitExpressionStmt(Expression &stmt) {
  resolve(stmt.expression);
  return std::any();
}

std::any Resolver::visitIfStmt(If &stmt) {
  resolve(stmt.condition);
  resolve(stmt.thenBranch);
  if (stmt.elseBranch != nullptr)
    resolve(stmt.elseBranch);
  return std::any();
}

std::any Resolver::visitPrintStmt(Print &stmt) {
  resolve(stmt.expression);
  return std::any();
}

std::any Resolver::visitReturnStmt(Return &stmt) {
  if (stmt.value != nullptr) {
    resolve(stmt.value);
  }
  return std::any();
}

std::any Resolver::visitWhileStmt(While &stmt) {
  resolve(stmt.condition);
  resolve(stmt.body);
  return std::any();
}

std::any Resolver::visitBinaryExpr(Binary &expr) {
  resolve(expr.left);
  resolve(expr.right);
  return std::any();
}

std::any Resolver::visitCallExpr(Call &expr) {
  resolve(expr.callee);
  for (const auto &arg : expr.arguments) {
    resolve(arg);
  }
  return std::any();
}

std::any Resolver::visitGroupingExpr(Grouping &expr) {
  resolve(expr.expression);
  return std::any();
}

std::any Resolver::visitLiteralExpr(Literal &expr) { return std::any(); }

std::any Resolver::visitLogicalExpr(Logical &expr) {
  resolve(expr.left);
  resolve(expr.right);
  return std::any();
}

std::any Resolver::visitUnaryExpr(Unary &expr) {
  resolve(expr.right);
  return std::any();
}

std::any Resolver::visitClassStmt(Class &stmt) {
  // Class da naam declare karo (Declare the class name)
  declare(stmt.name);
  define(stmt.name);

  // Agar superclass hai ta resolve karo (If there's a superclass, resolve it)
  if (stmt.superclass != nullptr) {
    // Check: class apne aap ton inherit nahi kar sakdi
    // (A class cannot inherit from itself)
    if (stmt.name.lexeme == stmt.superclass->name.lexeme) {
      std::cerr << "Error: A class can't inherit from itself." << std::endl;
    }
    resolve(std::make_shared<Variable>(stmt.superclass->name));
  }

  // Agar superclass hai ta "super" scope banao
  // (If superclass exists, create a scope for "super")
  if (stmt.superclass != nullptr) {
    beginScope();
    scopes.back()["super"] = true;
  }

  // "this" layi scope banao (Create scope for "this")
  beginScope();
  scopes.back()["this"] = true;

  // Har method resolve karo (Resolve each method)
  for (const auto &method : stmt.methods) {
    beginScope();
    for (const auto &param : method->params) {
      declare(param);
      define(param);
    }
    resolve(method->body);
    endScope();
  }

  endScope(); // "this" scope band karo

  // Agar superclass hai ta "super" scope band karo
  if (stmt.superclass != nullptr) {
    endScope();
  }

  return std::any();
}

std::any Resolver::visitGetExpr(Get &expr) {
  resolve(expr.object);
  return std::any();
}

std::any Resolver::visitSetExpr(Set &expr) {
  resolve(expr.value);
  resolve(expr.object);
  return std::any();
}

std::any Resolver::visitThisExpr(This &expr) {
  resolveLocal(&expr, expr.keyword);
  return std::any();
}

std::any Resolver::visitSuperExpr(Super &expr) {
  // "super" keyword nu resolve karo (Resolve the "super" keyword)
  resolveLocal(&expr, expr.keyword);
  return std::any();
}
