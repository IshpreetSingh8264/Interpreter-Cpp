#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include "Token.hpp"
#include <any>
#include <map>
#include <memory>
#include <string>

// Environment: Variables da ghar (Scope)
class Environment {
  std::shared_ptr<Environment> enclosing; // Papa scope
  std::map<std::string, std::any> values; // Maal

public:
  Environment() : enclosing(nullptr) {}
  Environment(std::shared_ptr<Environment> enclosing) : enclosing(enclosing) {}

  // Define: Nawa variable register karo
  void define(std::string name, std::any value);

  // Get: Variable labho
  std::any get(Token name);

  // Assign: Variable update karo
  void assign(Token name, std::any value);

  // Ancestor related logic (Scope hopping) will be added later if needed
};

#endif // ENVIRONMENT_HPP
