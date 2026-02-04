#ifndef LOXFUNCTION_HPP
#define LOXFUNCTION_HPP

#include "Environment.hpp"
#include "LoxCallable.hpp"
#include "Stmt.hpp"
#include <memory>

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

#endif // LOXFUNCTION_HPP
