#include "Interpreter.hpp"

#include "LoxClass.hpp"
#include "LoxFunction.hpp"
#include "LoxInstance.hpp"
#include "ReturnException.hpp"
#include "RuntimeError.hpp"
#include "value.hpp"

#include <any>
#include <iostream>
#include <map>
#include <memory>
#include <utility>

namespace lox {

// The statement half of the tree walk.
//
// Unlike the expression half, these methods do change state: visitVarStmt
// defines a binding, visitBlockStmt pushes a scope, visitClassStmt builds a
// class and a `super` binding. All of that goes through executeBlock or
// Environment, never through a raw pointer.

// --- Stmt Visitor ---

std::any Interpreter::visitExpressionStmt(Expression &stmt) {
  evaluate(stmt.expression);
  return std::any();
}

std::any Interpreter::visitPrintStmt(Print &stmt) {
  std::any value = evaluate(stmt.expression);
  std::cout << stringify(value) << std::endl;
  return std::any();
}

std::any Interpreter::visitVarStmt(Var &stmt) {
  // The initializer runs before the name exists, which is what lets
  // `var a = a;` in global scope read the outer `a`.
  std::any value;
  if (stmt.initializer != nullptr) {
    value = evaluate(stmt.initializer);
  }
  environment->define(stmt.name.lexeme, std::move(value));
  return std::any();
}

std::any Interpreter::visitBlockStmt(Block &stmt) {
  executeBlock(stmt.statements, std::make_shared<Environment>(environment));
  return std::any();
}

std::any Interpreter::visitIfStmt(If &stmt) {
  if (isTruthy(evaluate(stmt.condition))) {
    execute(stmt.thenBranch);
  } else if (stmt.elseBranch != nullptr) {
    execute(stmt.elseBranch);
  }
  return std::any();
}

std::any Interpreter::visitWhileStmt(While &stmt) {
  while (isTruthy(evaluate(stmt.condition))) {
    execute(stmt.body);
  }
  return std::any();
}

std::any Interpreter::visitFunctionStmt(Function &stmt) {
  // Capture environment for closure
  std::shared_ptr<LoxFunction> function =
      std::make_shared<LoxFunction>(stmt, environment);
  environment->define(stmt.name.lexeme, function);
  return std::any();
}

std::any Interpreter::visitReturnStmt(Return &stmt) {
  std::any value;
  if (stmt.value != nullptr) {
    value = evaluate(stmt.value);
  }

  // Unwind to the nearest LoxFunction::call. The Resolver has already rejected
  // a return outside a function, so the catch there is always reachable.
  throw ReturnException(std::move(value));
}

std::any Interpreter::visitClassStmt(Class &stmt) {
  // Superclass evaluate karo agar hai (Evaluate superclass if present)
  //
  // hasSuperclass is captured before superclass is moved into the LoxClass,
  // because the restore below has to know whether a `super` scope was pushed.
  const bool hasSuperclass = stmt.superclass != nullptr;
  std::shared_ptr<LoxClass> superclass;
  if (hasSuperclass) {
    // Use the resolved superclass Variable directly
    std::any superValue = evaluate(stmt.superclass);
    if (superValue.type() != typeid(std::shared_ptr<LoxClass>)) {
      throw RuntimeError(stmt.superclass->name, "Superclass must be a class.");
    }
    superclass = asClass(superValue);
  }

  // Class da naam define karo (Define the class name)
  //
  // The name is defined before the methods are built so a method body can
  // refer to its own class, and only then assigned the class itself. That is
  // what makes `class E {} class F < E {}` work when E is re-bound later.
  environment->define(stmt.name.lexeme, std::any());

  // Agar superclass hai ta nawa environment banao jis vich "super" hai
  // (If superclass exists, create new environment with "super")
  if (hasSuperclass) {
    environment = std::make_shared<Environment>(environment);
    environment->define("super", superclass);
  }

  // Methods nu bind karo (Bind the methods)
  std::map<std::string, std::shared_ptr<LoxFunction>> methods;
  for (const auto &method : stmt.methods) {
    // Check if this method is the initializer (constructor)
    bool isInitializer = (method->name.lexeme == "init");
    std::shared_ptr<LoxFunction> function =
        std::make_shared<LoxFunction>(*method, environment, isInitializer);
    methods[method->name.lexeme] = std::move(function);
  }

  // Class banao (Create the class)
  std::shared_ptr<LoxClass> klass = std::make_shared<LoxClass>(
      stmt.name.lexeme, std::move(superclass), std::move(methods));

  // Agar superclass hai ta environment wapas lao
  // (If superclass existed, restore environment)
  if (hasSuperclass) {
    environment = environment->enclosing;
  }

  environment->assign(stmt.name, klass);
  return std::any();
}

} // namespace lox
