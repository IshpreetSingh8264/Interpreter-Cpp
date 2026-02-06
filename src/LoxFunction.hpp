#ifndef LOX_LOXFUNCTION_HPP
#define LOX_LOXFUNCTION_HPP

#include "Environment.hpp"
#include "LoxCallable.hpp"
#include "Stmt.hpp"
#include <memory>

#include "lox.hpp"

namespace lox {

// Forward declaration
class LoxInstance;

// LoxFunction: User da banaya hoya function
// (LoxFunction: User-defined function)
class LoxFunction : public LoxCallable {
  // Function declaration AST node
  Function declaration;
  // Closure environment (Mahual captures kar liaya)
  // (Closure environment: Captured the vibe)
  std::shared_ptr<Environment> closure;
  // Is this function an initializer (constructor)?
  bool isInitializer;

public:
  LoxFunction(Function declaration, std::shared_ptr<Environment> closure,
              bool isInitializer = false)
      : declaration(declaration), closure(closure),
        isInitializer(isInitializer) {}

  int arity() override { return declaration.params.size(); }

  std::any call(Interpreter &interpreter,
                std::vector<std::any> arguments) override;

  std::shared_ptr<LoxFunction> bind(std::shared_ptr<LoxInstance> instance);

  bool getIsInitializer() const { return isInitializer; }

  std::string toString() override {
    return "<fn " + declaration.name.lexeme + ">";
  }
};

} // namespace lox

#endif // LOX_LOXFUNCTION_HPP
