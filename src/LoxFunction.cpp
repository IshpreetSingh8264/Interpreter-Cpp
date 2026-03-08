#include "LoxFunction.hpp"

#include "Interpreter.hpp"
#include "LoxInstance.hpp"
#include "ReturnException.hpp"

#include <memory>
#include <utility>

namespace lox {

LoxFunction::LoxFunction(Function declaration,
                         std::shared_ptr<Environment> closure,
                         bool isInitializer)
    : declaration(std::move(declaration)), closure(std::move(closure)),
      isInitializer(isInitializer) {}

int LoxFunction::arity() { return static_cast<int>(declaration.params.size()); }

std::shared_ptr<LoxFunction>
LoxFunction::bind(std::shared_ptr<LoxInstance> instance) {
  std::shared_ptr<Environment> environment =
      std::make_shared<Environment>(closure);
  environment->define("this", std::move(instance));
  // Bind layi isInitializer flag pass karo (Pass isInitializer flag for bind)
  return std::make_shared<LoxFunction>(declaration, environment, isInitializer);
}

std::any LoxFunction::call(Interpreter &interpreter,
                           std::vector<std::any> arguments) {
  // Create new environment with closure key (scope chain)
  // (Scope chain: Linking the environments together)
  std::shared_ptr<Environment> environment =
      std::make_shared<Environment>(closure);

  // Params nu arguments naal bind karo
  // (Bind params with arguments)
  // Arity was checked by the caller, so the two vectors are the same length.
  for (size_t i = 0; i < declaration.params.size(); ++i) {
    environment->define(declaration.params[i].lexeme, arguments[i]);
  }

  try {
    interpreter.executeBlock(declaration.body, environment);
  } catch (ReturnException &returnValue) {
    if (isInitializer) {
      // A bare `return;` in an initializer is legal and yields the instance. A
      // `return <value>;` is not, and the Resolver reports it - but the
      // Resolver does not run on the evaluate path, and it is the only thing
      // standing between `return 1;` and silently returning the instance and
      // dropping the value on the floor. Checking here too means the rule
      // holds however this function got called.
      if (returnValue.value.has_value()) {
        throw RuntimeError(returnValue.token,
                           "Can't return a value from an initializer.");
      }
      return thisInstance();
    }
    return returnValue.value;
  }

  // Agar initializer hai te "this" return karo
  // (If it's an initializer, return "this")
  if (isInitializer) {
    return thisInstance();
  }

  return std::any(); // nil return if no return statement
}

// An initializer's value is the instance it ran on, which bind() put at
// distance 0 of the closure. getSlotOrFail rather than getAt because a missing
// `this` here is the interpreter losing track, not nil.
std::any LoxFunction::thisInstance() const {
  return closure->getSlotOrFail(0, declaration.name, "this");
}

bool LoxFunction::getIsInitializer() const { return isInitializer; }

std::string LoxFunction::toString() { return "<fn " + declaration.name.lexeme + ">"; }

} // namespace lox
