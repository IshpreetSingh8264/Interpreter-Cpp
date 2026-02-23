#ifndef LOX_LOXCLASS_HPP
#define LOX_LOXCLASS_HPP

#include "LoxCallable.hpp"
#include "LoxFunction.hpp"
#include "LoxInstance.hpp"
#include <map>
#include <memory>
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
           std::map<std::string, std::shared_ptr<LoxFunction>> methods);

  std::string toString() override;
  int arity() override;

  std::any call(Interpreter &interpreter,
                std::vector<std::any> arguments) override;

  // Looks in this class, then walks up the superclass chain. Returns nullptr
  // when no class in the chain declares the method.
  std::shared_ptr<LoxFunction> findMethod(const std::string &name);
};

} // namespace lox

#endif // LOX_LOXCLASS_HPP
