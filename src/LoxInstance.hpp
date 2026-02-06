#ifndef LOX_LOXINSTANCE_HPP
#define LOX_LOXINSTANCE_HPP

#include "RuntimeError.hpp"
#include "Token.hpp"
#include <any>
#include <map>
#include <memory>
#include <string>

#include "lox.hpp"

namespace lox {

// Forward declaration
class LoxClass;

// LoxInstance: Class da bacha
// (LoxInstance: The child of the class)
class LoxInstance : public std::enable_shared_from_this<LoxInstance> {
  std::shared_ptr<LoxClass> klass;
  std::map<std::string, std::any> fields;

public:
  LoxInstance(std::shared_ptr<LoxClass> klass) : klass(klass) {}

  std::any get(Token name);
  void set(Token name, std::any value);
  std::string toString();
};

} // namespace lox

#endif // LOX_LOXINSTANCE_HPP
