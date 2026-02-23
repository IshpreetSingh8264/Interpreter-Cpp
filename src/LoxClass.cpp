#include "LoxClass.hpp"

#include "Interpreter.hpp"

#include <memory>
#include <utility>

namespace lox {

LoxClass::LoxClass(std::string name, std::shared_ptr<LoxClass> superclass,
                   std::map<std::string, std::shared_ptr<LoxFunction>> methods)
    : name(std::move(name)), superclass(std::move(superclass)),
      methods(std::move(methods)) {}

std::string LoxClass::toString() { return name; }

// Calling a class runs its initializer, so its arity is the initializer's
// arity. A class with no initializer takes no arguments.
int LoxClass::arity() {
  std::shared_ptr<LoxFunction> initializer = findMethod("init");
  if (initializer == nullptr) {
    return 0;
  }
  return initializer->arity();
}

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

std::shared_ptr<LoxFunction> LoxClass::findMethod(const std::string &name) {
  auto it = methods.find(name);
  if (it != methods.end()) {
    return it->second;
  }

  if (superclass != nullptr) {
    return superclass->findMethod(name);
  }

  return nullptr;
}

} // namespace lox
