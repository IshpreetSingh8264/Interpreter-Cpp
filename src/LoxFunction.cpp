#include "LoxFunction.hpp"
#include "Interpreter.hpp"
#include "LoxInstance.hpp"
#include "ReturnException.hpp"

#include "lox.hpp"

using namespace lox; // TEMP migration shim - removed at end of lox:: pass

std::shared_ptr<LoxFunction>
LoxFunction::bind(std::shared_ptr<LoxInstance> instance) {
  std::shared_ptr<Environment> environment =
      std::make_shared<Environment>(closure);
  environment->define("this", instance);
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
  for (size_t i = 0; i < declaration.params.size(); ++i) {
    environment->define(declaration.params[i].lexeme, arguments[i]);
  }

  try {
    interpreter.executeBlock(declaration.body, environment);
  } catch (ReturnException &returnValue) {
    // Agar initializer hai te return layi bhi "this" return karo
    // (If it's an initializer, return "this" even for return statements)
    if (isInitializer) {
      return closure->getAt(0, "this");
    }
    return returnValue.value;
  }

  // Agar initializer hai te "this" return karo
  // (If it's an initializer, return "this")
  if (isInitializer) {
    return closure->getAt(0, "this");
  }

  return std::any(); // nil return if no return statement
}
