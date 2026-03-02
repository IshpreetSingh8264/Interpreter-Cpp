#ifndef LOX_VALUE_HPP
#define LOX_VALUE_HPP

#include "RuntimeError.hpp"
#include "Token.hpp"

#include <any>
#include <memory>
#include <string>

#include "lox.hpp"

namespace lox {

class LoxClass;
class LoxFunction;
class LoxInstance;

// Lox has no tagged union, no variant and no null pointer. A runtime value is
// a std::any that is one of:
//
//   nil       an empty std::any
//   bool
//   double
//   std::string
//   shared_ptr<LoxClass>
//   shared_ptr<LoxInstance>
//   shared_ptr<LoxFunction>
//
//   ...plus any LoxCallable, which is how natives are stored.
//
// Every question the interpreter asks about a value - is it a number, what does
// it print as, are two of them equal, is it truthy - lives here rather than in
// the tree walker, so the answer to "what is a Lox value" is one file long.

// An empty std::any is nil. Everything else is the truth.
bool isTruthy(const std::any &value);

// Structural equality. nil equals nil; two values of different types are never
// equal, so (1 == "1") is false.
bool isEqual(const std::any &left, const std::any &right);

// The `print` representation of a value.
std::string stringify(const std::any &value);

// Operand checks. Both throw RuntimeError attributed to the operator token, so
// the reported line is the line the operator is on.
void requireNumber(const Token &operatorToken, const std::any &operand);
void requireNumbers(const Token &operatorToken, const std::any &left,
                    const std::any &right);

bool isNumber(const std::any &value);
bool isString(const std::any &value);
bool isBool(const std::any &value);

// Narrowing accessors. Each assumes the matching is* predicate passed, which is
// why they are unchecked.
double asNumber(const std::any &value);
std::string asString(const std::any &value);
bool asBool(const std::any &value);
std::shared_ptr<LoxClass> asClass(const std::any &value);
std::shared_ptr<LoxInstance> asInstance(const std::any &value);
std::shared_ptr<LoxFunction> asFunction(const std::any &value);

} // namespace lox

#endif // LOX_VALUE_HPP
