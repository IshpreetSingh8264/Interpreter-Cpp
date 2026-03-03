#include "AstPrinter.hpp"

#include <any>
#include <stdexcept>
#include <string>

namespace lox {
namespace {

// "a b c" -> "(a b c)"
std::string wrap(const std::string &body) { return "(" + body + ")"; }

// How a literal shows up in the AST.
//
// Note this is deliberately NOT value::stringify. The parse stages expect a
// number literal to keep one decimal place, so `57` renders as "57.0", while
// `print 57;` has to emit "57". The two commands format numbers differently,
// so the formatting lives here rather than being shared.
std::string literalText(const Literal &literal) {
  if (!literal.value.has_value()) {
    return "nil";
  }
  if (literal.value.type() == typeid(bool)) {
    return std::any_cast<bool>(literal.value) ? "true" : "false";
  }
  if (literal.value.type() == typeid(std::string)) {
    return std::any_cast<std::string>(literal.value);
  }
  if (literal.value.type() == typeid(double)) {
    std::string text = std::to_string(std::any_cast<double>(literal.value));
    size_t dot = text.find('.');
    if (dot != std::string::npos) {
      size_t last = text.find_last_not_of('0');
      text = last > dot ? text.substr(0, last + 1) : text.substr(0, dot + 2);
    }
    return text;
  }
  return "nil";
}

} // namespace

std::string AstPrinter::print(const std::shared_ptr<Expr> &expr) {
  AstPrinter printer;
  return printer.render(expr);
}

// The visitor methods return std::any only because ExprVisitor says so; render()
// unwraps it immediately so callers always get a string.
std::string AstPrinter::render(const std::shared_ptr<Expr> &expr) {
  if (!expr) {
    return "nil";
  }
  return text(expr->accept(*this));
}

// Every visit* method in this class returns a std::string, but ExprVisitor
// declares them as returning std::any. A node type added to the AST later would
// therefore fail to compile here rather than silently return nil - which is the
// whole point of making the printer a visitor.
std::string AstPrinter::text(const std::any &rendered) {
  const std::string *result = std::any_cast<std::string>(&rendered);
  if (result == nullptr) {
    throw std::runtime_error("AstPrinter: a visit method did not return text.");
  }
  return *result;
}

// --- renderings ---

std::any AstPrinter::visitLiteralExpr(Literal &expr) {
  return literalText(expr);
}

std::any AstPrinter::visitGroupingExpr(Grouping &expr) {
  return wrap("group " + render(expr.expression));
}

std::any AstPrinter::visitUnaryExpr(Unary &expr) {
  return wrap(expr.op.lexeme + " " + render(expr.right));
}

std::any AstPrinter::visitBinaryExpr(Binary &expr) {
  return wrap(expr.op.lexeme + " " + render(expr.left) + " " +
              render(expr.right));
}

// and/or are the same shape as the arithmetic operators, so they render the
// same way; the operator lexeme is what tells them apart.
std::any AstPrinter::visitLogicalExpr(Logical &expr) {
  return wrap(expr.op.lexeme + " " + render(expr.left) + " " +
              render(expr.right));
}

std::any AstPrinter::visitVariableExpr(Variable &expr) {
  return expr.name.lexeme;
}

std::any AstPrinter::visitAssignExpr(Assign &expr) {
  return wrap(expr.name.lexeme + " = " + render(expr.value));
}

// Note the explicit std::string: a bare "this" would land in the std::any as
// const char* and text() would reject it.
std::any AstPrinter::visitThisExpr(This &) { return std::string("this"); }

std::any AstPrinter::visitSuperExpr(Super &expr) {
  return wrap("super " + expr.method.lexeme);
}

std::any AstPrinter::visitGetExpr(Get &expr) {
  return wrap(". " + render(expr.object) + " " + expr.name.lexeme);
}

std::any AstPrinter::visitSetExpr(Set &expr) {
  return wrap(". " + render(expr.object) + " " + expr.name.lexeme + " = " +
              render(expr.value));
}

std::any AstPrinter::visitCallExpr(Call &expr) {
  std::string body = render(expr.callee);
  for (const auto &argument : expr.arguments) {
    body += " " + render(argument);
  }
  return wrap(body);
}

} // namespace lox
