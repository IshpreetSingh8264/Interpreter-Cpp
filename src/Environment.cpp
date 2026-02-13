#include "Environment.hpp"
#include "RuntimeError.hpp"
#include <iostream>

#include "lox.hpp"

namespace lox {

// Nawa variable register karo
// (Register a new variable)
void Environment::define(std::string name, std::any value) {
  values[name] = value;
}

// Variable labho
// (Find the variable)
std::any Environment::get(Token name) {
  if (values.count(name.lexeme)) {
    return values[name.lexeme];
  }

  if (enclosing != nullptr) {
    // Papa kol pucho
    return enclosing->get(name);
  }

  throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
}

// Variable assign karo
// (Assign the variable)
void Environment::assign(Token name, std::any value) {
  if (values.count(name.lexeme)) {
    values[name.lexeme] = value;
    return;
  }

  if (enclosing != nullptr) {
    // Papa nu kaho fix karn layi
    enclosing->assign(name, value);
    return;
  }

  throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
}

Environment *Environment::ancestor(int distance) {
  Environment *environment = this;
  for (int i = 0; i < distance; i++) {
    environment = environment->enclosing.get();
  }
  return environment;
}

std::any Environment::getAt(int distance, std::string name) {
  return ancestor(distance)->values[name];
}

void Environment::assignAt(int distance, Token name, std::any value) {
  ancestor(distance)->values[name.lexeme] = value;
}

} // namespace lox
