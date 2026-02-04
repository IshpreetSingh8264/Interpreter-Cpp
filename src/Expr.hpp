#ifndef EXPR_HPP
#define EXPR_HPP

#include "Token.hpp"
#include <any>
#include <memory>
#include <vector>

// Forward declarations
class Assign;
class Binary;
class Call;
class Grouping;
class Literal;
class Logical;
class Unary;
class Variable;

// Visitor Interface: Har expression nu milan da tarika
class ExprVisitor {
public:
  virtual std::any visitAssignExpr(Assign &expr) = 0;
  virtual std::any visitBinaryExpr(Binary &expr) = 0;
  virtual std::any visitCallExpr(Call &expr) = 0;
  virtual std::any visitGroupingExpr(Grouping &expr) = 0;
  virtual std::any visitLiteralExpr(Literal &expr) = 0;
  virtual std::any visitLogicalExpr(Logical &expr) = 0;
  virtual std::any visitUnaryExpr(Unary &expr) = 0;
  virtual std::any visitVariableExpr(Variable &expr) = 0;
  virtual ~ExprVisitor() = default;
};

// Base Expression class (Sab da baap)
class Expr {
public:
  virtual std::any accept(ExprVisitor &visitor) = 0;
  virtual ~Expr() = default;
};

// Subclasses (Bache)

class Binary : public Expr {
public:
  std::shared_ptr<Expr> left;
  Token op;
  std::shared_ptr<Expr> right;

  Binary(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right)
      : left(left), op(op), right(right) {}

  std::any accept(ExprVisitor &visitor) override {
    return visitor.visitBinaryExpr(*this);
  }
};

class Grouping : public Expr {
public:
  std::shared_ptr<Expr> expression;

  Grouping(std::shared_ptr<Expr> expression) : expression(expression) {}

  std::any accept(ExprVisitor &visitor) override {
    return visitor.visitGroupingExpr(*this);
  }
};

class Literal : public Expr {
public:
  std::any value;

  Literal(std::any value) : value(value) {}

  std::any accept(ExprVisitor &visitor) override {
    return visitor.visitLiteralExpr(*this);
  }
};

class Unary : public Expr {
public:
  Token op;
  std::shared_ptr<Expr> right;

  Unary(Token op, std::shared_ptr<Expr> right) : op(op), right(right) {}

  std::any accept(ExprVisitor &visitor) override {
    return visitor.visitUnaryExpr(*this);
  }
};

class Variable : public Expr {
public:
  Token name;

  Variable(Token name) : name(name) {}

  std::any accept(ExprVisitor &visitor) override {
    return visitor.visitVariableExpr(*this);
  }
};

class Assign : public Expr {
public:
  Token name;
  std::shared_ptr<Expr> value;

  Assign(Token name, std::shared_ptr<Expr> value) : name(name), value(value) {}

  std::any accept(ExprVisitor &visitor) override {
    return visitor.visitAssignExpr(*this);
  }
};

class Logical : public Expr {
public:
  std::shared_ptr<Expr> left;
  Token op;
  std::shared_ptr<Expr> right;

  Logical(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right)
      : left(left), op(op), right(right) {}

  std::any accept(ExprVisitor &visitor) override {
    return visitor.visitLogicalExpr(*this);
  }
};

class Call : public Expr {
public:
  std::shared_ptr<Expr> callee;
  Token paren;
  std::vector<std::shared_ptr<Expr>> arguments;

  Call(std::shared_ptr<Expr> callee, Token paren,
       std::vector<std::shared_ptr<Expr>> arguments)
      : callee(callee), paren(paren), arguments(arguments) {}

  std::any accept(ExprVisitor &visitor) override {
    return visitor.visitCallExpr(*this);
  }
};

#endif // EXPR_HPP
