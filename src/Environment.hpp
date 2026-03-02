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

  Environment();
  explicit Environment(std::shared_ptr<Environment> enclosing);

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
  //
  // getAt/assignAt walk exactly `distance` scopes up. They are unchecked
  // because they run on every variable read and the Resolver has already
  // proven the slot exists; a miss there would be an interpreter bug, and
  // fixing that on the hot path would cost every read in the program.
  std::any getAt(int distance, std::string name);
  void assignAt(int distance, Token name, std::any value);
  Environment *ancestor(int distance);

  // Looks up one of the interpreter's own well-known slots - "this" in a bound
  // method, "super" in a class scope - by name, `distance` scopes up, and
  // throws if the chain is too short or the slot is absent.
  //
  // Use this instead of getAt for those two slots: a miss means the
  // interpreter lost track of the scope chain rather than that the program is
  // wrong, and getAt would answer with a silently-inserted nil. `at` is the
  // token the error should point at, which is the `this`/`super` keyword in
  // the source rather than the slot being looked up.
  std::any getSlotOrFail(int distance, const Token &at, const char *slot);
};

} // namespace lox

#endif // LOX_ENVIRONMENT_HPP
