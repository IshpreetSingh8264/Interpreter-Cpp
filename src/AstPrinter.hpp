#ifndef LOX_ASTPRINTER_HPP
#define LOX_ASTPRINTER_HPP

#include "Expr.hpp"

#include <any>
#include <memory>
#include <string>

#include "lox.hpp"

namespace lox {

// Renders an expression back to Lox-ish source, the way the book's ASTPrinter
// does. This is what `./interpreter parse` prints.
//
// It is a full ExprVisitor rather than a dynamic_cast chain on purpose: the
// compiler then refuses to build until every expression node has a rendering,
// which is the defect this class was extracted to fix. The old chain handled
// five node types and returned the string "unknown" for the other seven, so
// `parse` silently produced no useful output for `and`, calls, property access,
// assignment, `this` and `super`.
class AstPrinter : public ExprVisitor {
public:
  static std::string print(const std::shared_ptr<Expr> &expr);

  std::any visitAssignExpr(Assign &expr) override;
  std::any visitBinaryExpr(Binary &expr) override;
  std::any visitCallExpr(Call &expr) override;
  std::any visitGetExpr(Get &expr) override;
  std::any visitGroupingExpr(Grouping &expr) override;
  std::any visitLiteralExpr(Literal &expr) override;
  std::any visitLogicalExpr(Logical &expr) override;
  std::any visitSetExpr(Set &expr) override;
  std::any visitSuperExpr(Super &expr) override;
  std::any visitThisExpr(This &expr) override;
  std::any visitUnaryExpr(Unary &expr) override;
  std::any visitVariableExpr(Variable &expr) override;

private:
  // A visitor method has to return std::any, but a printer wants a string, so
  // render() unwraps it via text().
  std::string render(const std::shared_ptr<Expr> &expr);
  static std::string text(const std::any &rendered);
};

} // namespace lox

#endif // LOX_ASTPRINTER_HPP
