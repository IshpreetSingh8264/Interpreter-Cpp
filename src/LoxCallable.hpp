#ifndef LOX_LOXCALLABLE_HPP
#define LOX_LOXCALLABLE_HPP

#include <any>
#include <memory>
#include <string>
#include <vector>

#include "lox.hpp"

namespace lox {

// Forward decl
class Interpreter;

// LoxCallable: Jo vi call ho skda hai (Functions, Classes)
// (LoxCallable: Anything that can be called, like Functions or Classes)
class LoxCallable {
public:
  virtual int arity() = 0; // Kinne arguments chahiye? (How many args needed?)
  virtual std::any
  call(Interpreter &interpreter,
       std::vector<std::any> arguments) = 0; // Call karo! (Do the deed!)
  virtual std::string toString() = 0;        // Naam ki hai? (What's your name?)
  virtual ~LoxCallable() = default;
};

} // namespace lox

#endif // LOX_LOXCALLABLE_HPP
