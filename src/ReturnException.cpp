#include "ReturnException.hpp"

#include <utility>

namespace lox {

ReturnException::ReturnException(std::any value, Token token)
    : std::runtime_error(""), value(std::move(value)), token(std::move(token)) {}

} // namespace lox
