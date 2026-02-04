#include "LoxInstance.hpp"
#include "LoxClass.hpp"
#include <iostream>

#include "lox.hpp"

using namespace lox; // TEMP migration shim - removed at end of lox:: pass

std::any LoxInstance::get(Token name) {
  if (fields.count(name.lexeme)) {
    return fields[name.lexeme];
  }

  std::shared_ptr<LoxFunction> method = klass->findMethod(name.lexeme);
  if (method != nullptr)
    return method->bind(shared_from_this());

  throw RuntimeError(name, "Undefined property '" + name.lexeme + "'.");
}

void LoxInstance::set(Token name, std::any value) {
  fields[name.lexeme] = value;
}

std::string LoxInstance::toString() { return klass->name + " instance"; }
