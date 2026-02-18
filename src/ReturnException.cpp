#include "ReturnException.hpp"

#include <utility>

namespace lox {

ReturnException::ReturnException(std::any value)
    : std::runtime_error(""), value(std::move(value)) {}

} // namespace lox
