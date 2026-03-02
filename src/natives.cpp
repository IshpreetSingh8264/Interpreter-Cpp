#include "natives.hpp"

#include "LoxCallable.hpp"
#include "LoxClass.hpp"
#include "LoxFunction.hpp"
#include "LoxInstance.hpp"
#include "RuntimeError.hpp"

#include <any>
#include <ctime>
#include <memory>
#include <string>
#include <vector>

namespace lox {
namespace {

// Native clock function - Returns Unix timestamp
// (Native clock function: Returns the current time as Unix timestamp!)
class Clock : public LoxCallable {
public:
  int arity() override { return 0; }

  std::any call(Interpreter &, std::vector<std::any> /*arguments*/) override {
    // Unix timestamp return karo (Return Unix timestamp)
    return static_cast<double>(std::time(nullptr));
  }

  std::string toString() override { return "<native fn>"; }
};

} // namespace

void installNatives(Environment &globals) {
  globals.define("clock",
                std::static_pointer_cast<LoxCallable>(std::make_shared<Clock>()));
}

} // namespace lox
