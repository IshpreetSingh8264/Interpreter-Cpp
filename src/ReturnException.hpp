#ifndef LOX_RETURNEXCEPTION_HPP
#define LOX_RETURNEXCEPTION_HPP

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
class ReturnException : public std::runtime_error {
public:
  std::any value;

  explicit ReturnException(std::any value);
};

} // namespace lox

#endif // LOX_RETURNEXCEPTION_HPP
