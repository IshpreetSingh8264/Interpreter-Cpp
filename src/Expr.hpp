#ifndef LOX_EXPR_HPP
#define LOX_EXPR_HPP

#include "Token.hpp"
#include <any>
#include <memory>
#include <vector>

#include "lox.hpp"


namespace lox {

// Forward declarations
class Assign;
class Binary;
class Call;
class Get;
class Grouping;
class Literal;
class Logical;
class Set;
class Super;
class This;
class Unary;
class Variable;

// Visitor Interface: Har expression nu milan da tarika
class ExprVisitor {
public:
  virtual std::any visitAssignExpr(Assign &expr) = 0;
  virtual std::any visitBinaryExpr(Binary &expr) = 0;
  virtual std::any visitCallExpr(Call &expr) = 0;
  virtual std::any visitGetExpr(Get &expr) = 0;
  virtual std::any visitGroupingExpr(Grouping &expr) = 0;
  virtual std::any visitLiteralExpr(Literal &expr) = 0;
  virtual std::any visitLogicalExpr(Logical &expr) = 0;
  virtual std::any visitSetExpr(Set &expr) = 0;
  virtual std::any visitSuperExpr(Super &expr) = 0;
  virtual std::any visitThisExpr(This &expr) = 0;
  virtual std::any visitUnaryExpr(Unary &expr) = 0;
  virtual std::any visitVariableExpr(Variable &expr) = 0;
  virtual ~ExprVisitor() = default;
};

// Base Expression class (Sab da baap)
// (Base Expression class: The father of them all)
class Expr {
public:
  virtual std::any accept(ExprVisitor &visitor) = 0;
  virtual ~Expr() = default;
};

// Subclasses (Bache)
// (Subclasses: The kids)

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

class Get : public Expr {
public:
  std::shared_ptr<Expr> object;
  Token name;

  Get(std::shared_ptr<Expr> object, Token name) : object(object), name(name) {}

  std::any accept(ExprVisitor &visitor) override {
    return visitor.visitGetExpr(*this);
  }
};

class Set : public Expr {
public:
  std::shared_ptr<Expr> object;
  Token name;
  std::shared_ptr<Expr> value;

  Set(std::shared_ptr<Expr> object, Token name, std::shared_ptr<Expr> value)
      : object(object), name(name), value(value) {}

  std::any accept(ExprVisitor &visitor) override {
    return visitor.visitSetExpr(*this);
  }
};

class This : public Expr {
public:
  Token keyword;

  This(Token keyword) : keyword(keyword) {}

  std::any accept(ExprVisitor &visitor) override {
    return visitor.visitThisExpr(*this);
  }
};

class Super : public Expr {
public:
  Token keyword;
  Token method;

  Super(Token keyword, Token method) : keyword(keyword), method(method) {}

  std::any accept(ExprVisitor &visitor) override {
    return visitor.visitSuperExpr(*this);
  }
};

} // namespace lox

#endif // LOX_EXPR_HPP
