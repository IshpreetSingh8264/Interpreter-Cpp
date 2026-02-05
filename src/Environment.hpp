#ifndef LOX_ENVIRONMENT_HPP
#define LOX_ENVIRONMENT_HPP

#include "Token.hpp"
#include <any>
#include <map>
#include <memory>
#include <string>

#include "lox.hpp"


namespace lox {

// Environment: Variables da ghar (Scope)
// (Environment: The home of variables, aka Scope)
class Environment {
  std::map<std::string, std::any> values; // Maal
  // (The goods: Where the values are stored)

public:
  std::shared_ptr<Environment> enclosing; // Papa scope (public for inheritance)
  // (Papa scope: The parent scope)

  Environment() : enclosing(nullptr) {}
  Environment(std::shared_ptr<Environment> enclosing) : enclosing(enclosing) {}

  // Define: Nawa variable register karo
  // (Define: Register a new variable)
  void define(std::string name, std::any value);

  // Get: Variable labho
  // (Get: Find that variable!)
  std::any get(Token name);

  // Assign: Variable update karo
  // (Assign: Update that variable!)
  void assign(Token name, std::any value);

  // Ancestor related logic (Scope hopping)
  std::any getAt(int distance, std::string name);
  void assignAt(int distance, Token name, std::any value);
  Environment *ancestor(int distance);
};

} // namespace lox

#endif // LOX_ENVIRONMENT_HPP
