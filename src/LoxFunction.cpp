#include "LoxFunction.hpp"
#include "Interpreter.hpp"
#include "LoxInstance.hpp"
#include "ReturnException.hpp"

std::shared_ptr<LoxFunction>
LoxFunction::bind(std::shared_ptr<LoxInstance> instance) {
  std::shared_ptr<Environment> environment =
      std::make_shared<Environment>(closure);
  environment->define("this", instance);
  return std::make_shared<LoxFunction>(declaration, environment);
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
    return returnValue.value;
  }

  return std::any(); // nil return if no return statement
}
