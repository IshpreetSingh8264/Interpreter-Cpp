#ifndef RETURNEXCEPTION_HPP
#define RETURNEXCEPTION_HPP

#include <any>
#include <stdexcept>

// Return exception: Wapis jao! (Control flow hack)
// (Return exception: Go back! The ultimate control flow hack)
class ReturnException : public std::runtime_error {
public:
  std::any value;

  ReturnException(std::any value) : std::runtime_error(""), value(value) {}
};

#endif // RETURNEXCEPTION_HPP
