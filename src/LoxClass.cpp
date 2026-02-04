#include "LoxClass.hpp"
#include "Interpreter.hpp"

std::any LoxClass::call(Interpreter &interpreter,
                        std::vector<std::any> arguments) {
  std::shared_ptr<LoxInstance> instance =
      std::make_shared<LoxInstance>(shared_from_this());

  std::shared_ptr<LoxFunction> initializer = findMethod("init");
  if (initializer != nullptr) {
    initializer->bind(instance)->call(interpreter, arguments);
  }

  return instance;
}

std::shared_ptr<LoxFunction> LoxClass::findMethod(std::string name) {
  if (methods.count(name)) {
    return methods[name];
  }

  if (superclass != nullptr) {
    return superclass->findMethod(name);
  }

  return nullptr;
}
