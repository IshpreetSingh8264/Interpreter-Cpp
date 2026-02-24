#include "LoxInstance.hpp"

#include "LoxClass.hpp"

#include <memory>
#include <utility>

namespace lox {

LoxInstance::LoxInstance(std::shared_ptr<LoxClass> klass)
    : klass(std::move(klass)) {}

std::any LoxInstance::get(Token name) {
  auto it = fields.find(name.lexeme);
  if (it != fields.end()) {
    return it->second;
  }

  std::shared_ptr<LoxFunction> method = klass->findMethod(name.lexeme);
  if (method != nullptr) {
    return method->bind(shared_from_this());
  }

  throw RuntimeError(name, "Undefined property '" + name.lexeme + "'.");
}

void LoxInstance::set(Token name, std::any value) {
  fields[name.lexeme] = std::move(value);
}

std::string LoxInstance::toString() { return klass->name + " instance"; }

} // namespace lox
