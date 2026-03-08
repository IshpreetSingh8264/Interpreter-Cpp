#ifndef LOX_RETURNEXCEPTION_HPP
#define LOX_RETURNEXCEPTION_HPP

#include "Token.hpp"

#include <any>
#include <stdexcept>

#include "lox.hpp"

namespace lox {

// Return exception: Wapis jao! (Control flow hack)
// (Return exception: Go back! The ultimate control flow hack)
//
// Lox's `return` unwinds the call stack without a return type on Function, so
// the value travels out as an exception. LoxFunction::call is the only place
// that catches it; nothing else should.
//
// The `return` keyword travels with it so an error raised while handling the
// return - "can't return a value from an initializer" - can point at the line
// the `return` is on rather than at the function's name.
class ReturnException : public std::runtime_error {
public:
  std::any value;
  Token token;

  ReturnException(std::any value, Token token);
};

} // namespace lox

#endif // LOX_RETURNEXCEPTION_HPP
