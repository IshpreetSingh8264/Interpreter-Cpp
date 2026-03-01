#ifndef LOX_RESOLVER_HPP
#define LOX_RESOLVER_HPP

#include "Expr.hpp"
#include "Interpreter.hpp"
#include "Stmt.hpp"
#include <map>
#include <string>
#include <vector>

#include "lox.hpp"

namespace lox {

// FunctionType enum for tracking current function context
enum class FunctionType { NONE, FUNCTION, INITIALIZER, METHOD };

// ClassType enum for tracking current class context
enum class ClassType { NONE, CLASS, SUBCLASS };

// Resolver: Static analysis pass (Rasta labho)
// (Resolver: Find the path!)
//
// Layer 3 of the pipeline. It walks the AST once before anything runs and
// records, for every variable reference, how many scopes up its binding lives.
// Those depths are handed to the Interpreter so it can reach a variable with
// getAt(depth, name) instead of searching the scope chain at runtime.
//
// Errors it reports are compile-time only: 'this' outside a class, 'super'
// outside a subclass, return at top level, a local read in its own
// initializer, and shadowing in the same scope.
class Resolver : public ExprVisitor, public StmtVisitor {
  Interpreter &interpreter;
  std::vector<std::map<std::string, bool>> scopes;
  FunctionType currentFunction = FunctionType::NONE;
  ClassType currentClass = ClassType::NONE;

public:
  bool hadError = false; // Error flag for compile-time errors

  explicit Resolver(Interpreter &interpreter);

  void resolve(const std::vector<std::shared_ptr<Stmt>> &statements);
  void resolve(std::shared_ptr<Stmt> stmt);
  void resolve(std::shared_ptr<Expr> expr);

private:
  void beginScope();
  void endScope();
  void declare(Token name);
  void define(Token name);
  void resolveLocal(Expr *expr, Token name);

  // Visitor implementations
  std::any visitBlockStmt(Block &stmt) override;
  std::any visitVarStmt(Var &stmt) override;
  std::any visitFunctionStmt(Function &stmt) override;
  std::any visitExpressionStmt(Expression &stmt) override;
  std::any visitIfStmt(If &stmt) override;
  std::any visitPrintStmt(Print &stmt) override;
  std::any visitReturnStmt(Return &stmt) override;
  std::any visitWhileStmt(While &stmt) override;
  std::any visitClassStmt(Class &stmt) override;

  std::any visitVariableExpr(Variable &expr) override;
  std::any visitAssignExpr(Assign &expr) override;
  std::any visitBinaryExpr(Binary &expr) override;
  std::any visitCallExpr(Call &expr) override;
  std::any visitGetExpr(Get &expr) override;
  std::any visitSetExpr(Set &expr) override;
  std::any visitThisExpr(This &expr) override;
  std::any visitSuperExpr(Super &expr) override;
  std::any visitGroupingExpr(Grouping &expr) override;
  std::any visitLiteralExpr(Literal &expr) override;
  std::any visitLogicalExpr(Logical &expr) override;
  std::any visitUnaryExpr(Unary &expr) override;
};

} // namespace lox

#endif // LOX_RESOLVER_HPP
