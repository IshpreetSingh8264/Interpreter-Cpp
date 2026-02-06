#ifndef LOX_LOXCLASS_HPP
#define LOX_LOXCLASS_HPP

#include "LoxCallable.hpp"
#include "LoxFunction.hpp"
#include "LoxInstance.hpp"
#include <map>
#include <string>
#include <vector>

#include "lox.hpp"

namespace lox {

// LoxClass: Class di definiton
// (LoxClass: Definition of the class)
class LoxClass : public LoxCallable,
                 public std::enable_shared_from_this<LoxClass> {
public:
  std::string name;
  std::shared_ptr<LoxClass> superclass;
  std::map<std::string, std::shared_ptr<LoxFunction>> methods;

  LoxClass(std::string name, std::shared_ptr<LoxClass> superclass,
           std::map<std::string, std::shared_ptr<LoxFunction>> methods)
      : name(name), superclass(superclass), methods(methods) {}

  std::string toString() override { return name; }

  int arity() override {
    std::shared_ptr<LoxFunction> initializer = findMethod("init");
    if (initializer == nullptr)
      return 0;
    return initializer->arity();
  }

  std::any call(Interpreter &interpreter,
                std::vector<std::any> arguments) override;

  std::shared_ptr<LoxFunction> findMethod(std::string name);
};

} // namespace lox

#endif // LOX_LOXCLASS_HPP
