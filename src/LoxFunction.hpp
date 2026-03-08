#ifndef LOX_LOXFUNCTION_HPP
#define LOX_LOXFUNCTION_HPP

#include "Environment.hpp"
#include "LoxCallable.hpp"
#include "Stmt.hpp"
#include <memory>
#include <string>

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
              bool isInitializer = false);

  int arity() override;
  std::any call(Interpreter &interpreter,
                std::vector<std::any> arguments) override;

  // Returns a copy of this function whose closure defines `this`. This is how
  // a method and `super.method` both get the instance they run against.
  std::shared_ptr<LoxFunction> bind(std::shared_ptr<LoxInstance> instance);

  bool getIsInitializer() const;
  std::string toString() override;

private:
  // The instance an initializer returns: the `this` bind() put at distance 0 of
  // the closure.
  std::any thisInstance() const;
};

} // namespace lox

#endif // LOX_LOXFUNCTION_HPP
