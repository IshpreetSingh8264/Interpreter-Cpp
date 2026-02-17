#include "Stmt.hpp"

#include <memory>
#include <utility>

namespace lox {

Block::Block(std::vector<std::shared_ptr<Stmt>> statements)
    : statements(std::move(statements)) {}

std::any Block::accept(StmtVisitor &visitor) { return visitor.visitBlockStmt(*this); }

Expression::Expression(std::shared_ptr<Expr> expression)
    : expression(std::move(expression)) {}

std::any Expression::accept(StmtVisitor &visitor) {
  return visitor.visitExpressionStmt(*this);
}

Class::Class(Token name, std::shared_ptr<Variable> superclass,
             std::vector<std::shared_ptr<Function>> methods)
    : name(name), superclass(std::move(superclass)), methods(std::move(methods)) {}

std::any Class::accept(StmtVisitor &visitor) { return visitor.visitClassStmt(*this); }

Function::Function(Token name, std::vector<Token> params,
                   std::vector<std::shared_ptr<Stmt>> body)
    : name(name), params(std::move(params)), body(std::move(body)) {}

std::any Function::accept(StmtVisitor &visitor) {
  return visitor.visitFunctionStmt(*this);
}

If::If(std::shared_ptr<Expr> condition, std::shared_ptr<Stmt> thenBranch,
       std::shared_ptr<Stmt> elseBranch)
    : condition(std::move(condition)), thenBranch(std::move(thenBranch)),
      elseBranch(std::move(elseBranch)) {}

std::any If::accept(StmtVisitor &visitor) { return visitor.visitIfStmt(*this); }

Print::Print(std::shared_ptr<Expr> expression)
    : expression(std::move(expression)) {}

std::any Print::accept(StmtVisitor &visitor) { return visitor.visitPrintStmt(*this); }

Return::Return(Token keyword, std::shared_ptr<Expr> value)
    : keyword(keyword), value(std::move(value)) {}

std::any Return::accept(StmtVisitor &visitor) {
  return visitor.visitReturnStmt(*this);
}

Var::Var(Token name, std::shared_ptr<Expr> initializer)
    : name(name), initializer(std::move(initializer)) {}

std::any Var::accept(StmtVisitor &visitor) { return visitor.visitVarStmt(*this); }

While::While(std::shared_ptr<Expr> condition, std::shared_ptr<Stmt> body)
    : condition(std::move(condition)), body(std::move(body)) {}

std::any While::accept(StmtVisitor &visitor) { return visitor.visitWhileStmt(*this); }

} // namespace lox
