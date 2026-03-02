#ifndef LOX_INTERPRETER_HPP
#define LOX_INTERPRETER_HPP

#include "Environment.hpp"
#include "Expr.hpp"
#include "Stmt.hpp"

#include <any>
#include <map>
#include <memory>
#include <vector>

#include "lox.hpp"

namespace lox {

// Interpreter: Asli hero jo code chalaunda hai
// (Interpreter: The real hero that runs the code)
//
// Layer 4 of the pipeline and the last one. It walks an AST that the Resolver
// has already annotated, holding the current scope in `environment` and the
// resolution depths in `locals`.
//
// This header is the class contract only. The implementation is split by
// concern so no single file holds all of it:
//
//   Interpreter.cpp          construction, interpret/evaluate/execute, scoping
//   visit_expression.cpp     the twelve ExprVisitor methods
//   visit_statement.cpp      the nine StmtVisitor methods
//   value.{hpp,cpp}          Lox value semantics: truthiness, equality,
//                            printing, operand checks
//   natives.{hpp,cpp}        the built-in functions installed into globals
class Interpreter : public ExprVisitor, public StmtVisitor {
public:
  Interpreter();

  std::shared_ptr<Environment> globals;
  std::shared_ptr<Environment> environment;

  // Locals map (Resolution info)
  // (Locals map: Where do I live?)
  //
  // Filled in by the Resolver before interpret() runs: every Expr* that binds
  // to a local gets the number of scopes up its binding lives. An Expr* that is
  // absent is a global. Keyed by pointer, so the AST has to outlive this map.
  std::map<Expr *, int> locals;

  // Resolve scope depth
  // (Resolve scope depth: How deep is your love... for variables?)
  void resolve(Expr *expr, int depth);

  // Main entry point
  // Tokens/Statements phado te chalao
  // (Grab the tokens/statements and run 'em!)
  void interpret(const std::vector<std::shared_ptr<Stmt>> &statements);

  // Evaluate expression helper (Used for testing/REPL)
  std::any evaluate(std::shared_ptr<Expr> expr);

  // Execute Block (Public for LoxFunction)
  // (Execute Block: Public access for LoxFunction)
  //
  // Runs statements in a nested environment and restores the previous one even
  // if a statement unwinds through here.
  void executeBlock(const std::vector<std::shared_ptr<Stmt>> &statements,
                    std::shared_ptr<Environment> environment);

  // Visitor Implementations (Expr) - defined in visit_expression.cpp
  std::any visitLiteralExpr(Literal &expr) override;
  std::any visitGroupingExpr(Grouping &expr) override;
  std::any visitUnaryExpr(Unary &expr) override;
  std::any visitBinaryExpr(Binary &expr) override;
  std::any visitVariableExpr(Variable &expr) override;
  std::any visitAssignExpr(Assign &expr) override;
  std::any visitLogicalExpr(Logical &expr) override;
  std::any visitCallExpr(Call &expr) override;
  std::any visitGetExpr(Get &expr) override;
  std::any visitSetExpr(Set &expr) override;
  std::any visitThisExpr(This &expr) override;
  std::any visitSuperExpr(Super &expr) override;

  // Visitor Implementations (Stmt) - defined in visit_statement.cpp
  std::any visitExpressionStmt(Expression &stmt) override;
  std::any visitPrintStmt(Print &stmt) override;
  std::any visitVarStmt(Var &stmt) override;
  std::any visitBlockStmt(Block &stmt) override;
  std::any visitIfStmt(If &stmt) override;
  std::any visitWhileStmt(While &stmt) override;
  std::any visitFunctionStmt(Function &stmt) override;
  std::any visitReturnStmt(Return &stmt) override;
  std::any visitClassStmt(Class &stmt) override;

private:
  void execute(std::shared_ptr<Stmt> stmt);
};

} // namespace lox

#endif // LOX_INTERPRETER_HPP
