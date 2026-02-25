#include "Environment.hpp"

#include "RuntimeError.hpp"

#include <utility>

namespace lox {

Environment::Environment() : enclosing(nullptr) {}

Environment::Environment(std::shared_ptr<Environment> enclosing)
    : enclosing(std::move(enclosing)) {}

// Nawa variable register karo
// (Register a new variable)
void Environment::define(std::string name, std::any value) {
  values[std::move(name)] = std::move(value);
}

// Variable labho
// (Find the variable)
std::any Environment::get(Token name) {
  auto it = values.find(name.lexeme);
  if (it != values.end()) {
    return it->second;
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
  auto it = values.find(name.lexeme);
  if (it != values.end()) {
    it->second = std::move(value);
    return;
  }

  if (enclosing != nullptr) {
    // Papa nu kaho fix karn layi
    enclosing->assign(name, std::move(value));
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
  return ancestor(distance)->values[std::move(name)];
}

void Environment::assignAt(int distance, Token name, std::any value) {
  ancestor(distance)->values[std::move(name.lexeme)] = std::move(value);
}

} // namespace lox
