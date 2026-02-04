#ifndef LOX_RETURNEXCEPTION_HPP
#define LOX_RETURNEXCEPTION_HPP

#include <any>
#include <stdexcept>

#include "lox.hpp"


namespace lox {

// Return exception: Wapis jao! (Control flow hack)
// (Return exception: Go back! The ultimate control flow hack)
class ReturnException : public std::runtime_error {
public:
  std::any value;

  ReturnException(std::any value) : std::runtime_error(""), value(value) {}
};

} // namespace lox

#endif // LOX_RETURNEXCEPTION_HPP
