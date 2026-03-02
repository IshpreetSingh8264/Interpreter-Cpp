#include "value.hpp"

#include "LoxCallable.hpp"
#include "LoxClass.hpp"
#include "LoxFunction.hpp"
#include "LoxInstance.hpp"

#include <string>

namespace lox {

bool isNumber(const std::any &value) {
  return value.type() == typeid(double);
}

bool isString(const std::any &value) {
  return value.type() == typeid(std::string);
}

bool isBool(const std::any &value) { return value.type() == typeid(bool); }

double asNumber(const std::any &value) { return std::any_cast<double>(value); }

std::string asString(const std::any &value) {
  return std::any_cast<std::string>(value);
}

bool asBool(const std::any &value) { return std::any_cast<bool>(value); }

std::shared_ptr<LoxClass> asClass(const std::any &value) {
  return std::any_cast<std::shared_ptr<LoxClass>>(value);
}

std::shared_ptr<LoxInstance> asInstance(const std::any &value) {
  return std::any_cast<std::shared_ptr<LoxInstance>>(value);
}

std::shared_ptr<LoxFunction> asFunction(const std::any &value) {
  return std::any_cast<std::shared_ptr<LoxFunction>>(value);
}

bool isTruthy(const std::any &value) {
  if (!value.has_value()) {
    return false; // nil
  }
  if (isBool(value)) {
    return asBool(value);
  }
  return true; // everything else is true
}

bool isEqual(const std::any &left, const std::any &right) {
  if (!left.has_value() && !right.has_value()) {
    return true;
  }
  if (!left.has_value() || !right.has_value()) {
    return false;
  }

  if (isBool(left) && isBool(right)) {
    return asBool(left) == asBool(right);
  }
  if (isNumber(left) && isNumber(right)) {
    return asNumber(left) == asNumber(right);
  }
  if (isString(left) && isString(right)) {
    return asString(left) == asString(right);
  }

  return false;
}

std::string stringify(const std::any &value) {
  if (!value.has_value()) {
    return "nil";
  }

  if (isNumber(value)) {
    // Lox has one numeric type, so a whole number and the same number written
    // with a decimal point print identically: 72 prints as "72", not "72.0".
    // std::to_string always emits six decimal places, so trim them back.
    std::string text = std::to_string(asNumber(value));
    if (text.find('.') != std::string::npos) {
      text = text.substr(0, text.find_last_not_of('0') + 1);
      if (text.back() == '.') {
        text.pop_back();
      }
    }
    return text;
  }

  if (isString(value)) {
    return asString(value);
  }
  if (isBool(value)) {
    return asBool(value) ? "true" : "false";
  }
  if (value.type() == typeid(std::shared_ptr<LoxClass>)) {
    return asClass(value)->toString();
  }
  if (value.type() == typeid(std::shared_ptr<LoxInstance>)) {
    return asInstance(value)->toString();
  }
  if (value.type() == typeid(std::shared_ptr<LoxFunction>)) {
    return asFunction(value)->toString();
  }

  // The only value type left is a native, which is how `clock` prints.
  if (value.type() == typeid(std::shared_ptr<LoxCallable>)) {
    return std::any_cast<std::shared_ptr<LoxCallable>>(value)->toString();
  }

  throw RuntimeError(Token(TokenType::END_OF_FILE, "", std::any(), 0),
                     "Cannot stringify a value of this type.");
}

void requireNumber(const Token &operatorToken, const std::any &operand) {
  if (isNumber(operand)) {
    return;
  }
  throw RuntimeError(operatorToken, "Operand must be a number.");
}

void requireNumbers(const Token &operatorToken, const std::any &left,
                    const std::any &right) {
  if (isNumber(left) && isNumber(right)) {
    return;
  }
  throw RuntimeError(operatorToken, "Operands must be numbers.");
}

} // namespace lox
