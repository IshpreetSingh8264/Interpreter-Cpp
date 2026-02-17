#include "Expr.hpp"

#include <memory>
#include <utility>

namespace lox {

// Each node owns two things: its children and the name of the visitor method
// that knows how to handle it. Keeping accept() out of line means adding an
// expression node is a two-file change with no risk of a stale inline copy.

Binary::Binary(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right)
    : left(std::move(left)), op(op), right(std::move(right)) {}

std::any Binary::accept(ExprVisitor &visitor) { return visitor.visitBinaryExpr(*this); }

Grouping::Grouping(std::shared_ptr<Expr> expression)
    : expression(std::move(expression)) {}

std::any Grouping::accept(ExprVisitor &visitor) {
  return visitor.visitGroupingExpr(*this);
}

Literal::Literal(std::any value) : value(std::move(value)) {}

std::any Literal::accept(ExprVisitor &visitor) {
  return visitor.visitLiteralExpr(*this);
}

Unary::Unary(Token op, std::shared_ptr<Expr> right)
    : op(op), right(std::move(right)) {}

std::any Unary::accept(ExprVisitor &visitor) { return visitor.visitUnaryExpr(*this); }

Variable::Variable(Token name) : name(name) {}

std::any Variable::accept(ExprVisitor &visitor) {
  return visitor.visitVariableExpr(*this);
}

Assign::Assign(Token name, std::shared_ptr<Expr> value)
    : name(name), value(std::move(value)) {}

std::any Assign::accept(ExprVisitor &visitor) { return visitor.visitAssignExpr(*this); }

Logical::Logical(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right)
    : left(std::move(left)), op(op), right(std::move(right)) {}

std::any Logical::accept(ExprVisitor &visitor) {
  return visitor.visitLogicalExpr(*this);
}

Call::Call(std::shared_ptr<Expr> callee, Token paren,
           std::vector<std::shared_ptr<Expr>> arguments)
    : callee(std::move(callee)), paren(paren), arguments(std::move(arguments)) {}

std::any Call::accept(ExprVisitor &visitor) { return visitor.visitCallExpr(*this); }

Get::Get(std::shared_ptr<Expr> object, Token name)
    : object(std::move(object)), name(name) {}

std::any Get::accept(ExprVisitor &visitor) { return visitor.visitGetExpr(*this); }

Set::Set(std::shared_ptr<Expr> object, Token name, std::shared_ptr<Expr> value)
    : object(std::move(object)), name(name), value(std::move(value)) {}

std::any Set::accept(ExprVisitor &visitor) { return visitor.visitSetExpr(*this); }

This::This(Token keyword) : keyword(keyword) {}

std::any This::accept(ExprVisitor &visitor) { return visitor.visitThisExpr(*this); }

Super::Super(Token keyword, Token method) : keyword(keyword), method(method) {}

std::any Super::accept(ExprVisitor &visitor) { return visitor.visitSuperExpr(*this); }

} // namespace lox
