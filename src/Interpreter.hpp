#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include "Environment.hpp"
#include "Expr.hpp"
#include "Stmt.hpp"
#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include "lox.hpp"

using namespace lox; // TEMP migration shim - removed at end of lox:: pass

// Interpreter: Asli hero jo code chalaunda hai
// (Interpreter: The real hero that runs the code)
class Interpreter : public ExprVisitor, public StmtVisitor {
public:
  std::shared_ptr<Environment> globals;
  std::shared_ptr<Environment> environment;

  Interpreter();

  // Locals map (Resolution info)
  // (Locals map: Where do I live?)
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
  void executeBlock(const std::vector<std::shared_ptr<Stmt>> &statements,
                    std::shared_ptr<Environment> environment);

  // Helper to stringify result (Public for REPL)
  std::string stringify(std::any object);

  // Visitor Implementations (Expr)
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

  // Visitor Implementations (Stmt)
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

  // Helpers
  // (Helpers)
  bool isTruthy(std::any object);
  bool isEqual(std::any a, std::any b);
  void checkNumberOperand(Token operatorToken, std::any operand);
  void checkNumberOperands(Token operatorToken, std::any left, std::any right);
};

#endif // INTERPRETER_HPP
