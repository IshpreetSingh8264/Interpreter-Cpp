#ifndef LOX_STMT_HPP
#define LOX_STMT_HPP

#include "Expr.hpp"
#include "Token.hpp"
#include <any>
#include <memory>
#include <vector>

#include "lox.hpp"


namespace lox {

// Forward declarations
class Block;
class Class;
class Expression;
class Function;
class If;
class Print;
class Return;
class Var;
class While;

// Visitor Interface: Har statement nu milan da tarika
// (Visitor Interface: The protocol for meeting every statement)
class StmtVisitor {
public:
  virtual std::any visitBlockStmt(Block &stmt) = 0;
  virtual std::any visitClassStmt(Class &stmt) = 0;
  virtual std::any visitExpressionStmt(Expression &stmt) = 0;
  virtual std::any visitFunctionStmt(Function &stmt) = 0;
  virtual std::any visitIfStmt(If &stmt) = 0;
  virtual std::any visitPrintStmt(Print &stmt) = 0;
  virtual std::any visitReturnStmt(Return &stmt) = 0;
  virtual std::any visitVarStmt(Var &stmt) = 0;
  virtual std::any visitWhileStmt(While &stmt) = 0;
  virtual ~StmtVisitor() = default;
};

// Base Statement class (Hukam da baap)
// (Base Statement class: The father of commands)
class Stmt {
public:
  virtual std::any accept(StmtVisitor &visitor) = 0;
  virtual ~Stmt() = default;
};

// Subclasses (Vakhre vakhre hukam)
// (Subclasses: different different orders)

class Block : public Stmt {
public:
  std::vector<std::shared_ptr<Stmt>> statements;

  Block(std::vector<std::shared_ptr<Stmt>> statements)
      : statements(statements) {}

  std::any accept(StmtVisitor &visitor) override {
    return visitor.visitBlockStmt(*this);
  }
};

class Expression : public Stmt {
public:
  std::shared_ptr<Expr> expression;

  Expression(std::shared_ptr<Expr> expression) : expression(expression) {}

  std::any accept(StmtVisitor &visitor) override {
    return visitor.visitExpressionStmt(*this);
  }
};

class Class : public Stmt {
public:
  Token name;
  std::shared_ptr<Variable> superclass; // Inheritance
  std::vector<std::shared_ptr<Function>> methods;

  Class(Token name, std::shared_ptr<Variable> superclass,
        std::vector<std::shared_ptr<Function>> methods)
      : name(name), superclass(superclass), methods(methods) {}

  std::any accept(StmtVisitor &visitor) override {
    return visitor.visitClassStmt(*this);
  }
};

class Function : public Stmt {
public:
  Token name;
  std::vector<Token> params;
  std::vector<std::shared_ptr<Stmt>> body;

  Function(Token name, std::vector<Token> params,
           std::vector<std::shared_ptr<Stmt>> body)
      : name(name), params(params), body(body) {}

  std::any accept(StmtVisitor &visitor) override {
    return visitor.visitFunctionStmt(*this);
  }
};

class If : public Stmt {
public:
  std::shared_ptr<Expr> condition;
  std::shared_ptr<Stmt> thenBranch;
  std::shared_ptr<Stmt> elseBranch;

  If(std::shared_ptr<Expr> condition, std::shared_ptr<Stmt> thenBranch,
     std::shared_ptr<Stmt> elseBranch)
      : condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {}

  std::any accept(StmtVisitor &visitor) override {
    return visitor.visitIfStmt(*this);
  }
};

class Print : public Stmt {
public:
  std::shared_ptr<Expr> expression;

  Print(std::shared_ptr<Expr> expression) : expression(expression) {}

  std::any accept(StmtVisitor &visitor) override {
    return visitor.visitPrintStmt(*this);
  }
};

class Return : public Stmt {
public:
  Token keyword;
  std::shared_ptr<Expr> value;

  Return(Token keyword, std::shared_ptr<Expr> value)
      : keyword(keyword), value(value) {}

  std::any accept(StmtVisitor &visitor) override {
    return visitor.visitReturnStmt(*this);
  }
};

class Var : public Stmt {
public:
  Token name;
  std::shared_ptr<Expr> initializer;

  Var(Token name, std::shared_ptr<Expr> initializer)
      : name(name), initializer(initializer) {}

  std::any accept(StmtVisitor &visitor) override {
    return visitor.visitVarStmt(*this);
  }
};

class While : public Stmt {
public:
  std::shared_ptr<Expr> condition;
  std::shared_ptr<Stmt> body;

  While(std::shared_ptr<Expr> condition, std::shared_ptr<Stmt> body)
      : condition(condition), body(body) {}

  std::any accept(StmtVisitor &visitor) override {
    return visitor.visitWhileStmt(*this);
  }
};

} // namespace lox

#endif // LOX_STMT_HPP
