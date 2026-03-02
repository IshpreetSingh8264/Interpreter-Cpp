#include "Interpreter.hpp"

#include "Environment.hpp"
#include "natives.hpp"

#include <memory>

namespace lox {

// This TU owns the Interpreter's own machinery: construction, the three entry
// points, and scope switching. The visitor methods live in visit_expression.cpp
// and visit_statement.cpp; the questions about Lox values live in value.cpp.

Interpreter::Interpreter() {
  globals = std::make_shared<Environment>();
  environment = globals;
  installNatives(*globals);
}

// Main entry point
void Interpreter::interpret(
    const std::vector<std::shared_ptr<Stmt>> &statements) {
  // Error handling main vich hovega (Main banda fix karega)
  // (Error handling will be in main, the main boss will fix it)
  for (const auto &stmt : statements) {
    if (stmt) {
      execute(stmt);
    }
  }
}

// Called by the Resolver, before anything runs, to record how many scopes up a
// variable's binding lives.
void Interpreter::resolve(Expr *expr, int depth) { locals[expr] = depth; }

std::any Interpreter::evaluate(std::shared_ptr<Expr> expr) {
  return expr->accept(*this);
}

void Interpreter::execute(std::shared_ptr<Stmt> stmt) { stmt->accept(*this); }

void Interpreter::executeBlock(
    const std::vector<std::shared_ptr<Stmt>> &statements,
    std::shared_ptr<Environment> environment) {
  std::shared_ptr<Environment> previous = this->environment;

  try {
    this->environment = std::move(environment); // Scope switch (Ghar badlo)
                                                    // (Scope switch: Change houses)

    for (const auto &stmt : statements) {
      execute(stmt);
    }
  } catch (...) {
    this->environment = previous; // Wapis purane ghar
                                  // (Back to the old house)
    throw;
  }
  this->environment = previous;
}

} // namespace lox
