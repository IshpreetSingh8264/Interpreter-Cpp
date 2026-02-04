#include "Environment.hpp"
#include "RuntimeError.hpp"
#include <iostream>

// Nawa variable register karo
void Environment::define(std::string name, std::any value) {
  values[name] = value;
}

// Variable labho
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
