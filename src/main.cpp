#include "AstPrinter.hpp"
#include "Expr.hpp"
#include "Interpreter.hpp"
#include "Parser.hpp"
#include "Resolver.hpp"
#include "RuntimeError.hpp"
#include "Scanner.hpp"
#include "Stmt.hpp"
#include "Token.hpp"
#include "TokenType.hpp"
#include "value.hpp"

#include <any>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "lox.hpp"

namespace lox {

// Utility to read file contents
std::string read_file_contents(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Error reading file: " << filename << std::endl;
    std::exit(1);
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  file.close();

  return buffer.str();
}

} // namespace lox

int main(int argc, char *argv[]) {
  // Everything the interpreter is made of lives in lox. main() is the one
  // symbol that has to stay at global scope, so it reaches into the namespace
  // rather than the other way round.
  using namespace lox;

  // Disable output buffering
  // Direct connection, no delay!
  // (Direct connection, no waiting room!)
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  if (argc < 3) {
    std::cerr << "Usage: ./your_program <command> <filename>" << std::endl;
    return 1;
  }

  const std::string command = argv[1];

  if (command == "tokenize") {
    std::string file_contents = read_file_contents(argv[2]);

    // Scanner nu bulao
    // (Call the Scanner!)
    Scanner scanner(file_contents);
    std::vector<Token> tokens = scanner.scanTokens();

    // Tokens nu print karo
    // (Print those tokens!)
    for (const Token &token : tokens) {
      std::cout << token.toString() << std::endl;
    }

    // Agar koi panga peya si ta dasso
    // (If there was any trouble, spill the beans)
    if (scanner.hasError)
      return 65;
  } else if (command == "parse") {
    std::string file_contents = read_file_contents(argv[2]);
    Scanner scanner(file_contents);
    std::vector<Token> tokens = scanner.scanTokens();
    if (scanner.hasError)
      return 65;
    Parser parser(tokens);
    // Parser sirf expression parse karo (Parse only expression for
    // Codecrafters)
    std::shared_ptr<Expr> expression = parser.parseExpression();
    if (parser.hadError)
      return 65;
    if (expression) {
      // AST nu print karo (Print the AST)
      std::cout << AstPrinter::print(expression) << std::endl;
    }
  } else if (command == "evaluate") {
    std::string file_contents = read_file_contents(argv[2]);
    Scanner scanner(file_contents);
    std::vector<Token> tokens = scanner.scanTokens();
    Parser parser(tokens);
    std::shared_ptr<Expr> expression = parser.parseExpression();
    if (parser.hadError)
      return 65;
    if (expression) {
      Interpreter interpreter;
      try {
        std::any result = interpreter.evaluate(expression);
        std::cout << stringify(result) << std::endl;
      } catch (RuntimeError &error) {
        std::cerr << error.what() << "\n[line " << error.token.line << "]"
                  << std::endl;
        return 70;
      }
    }
  } else if (command == "run") {
    std::string file_contents = read_file_contents(argv[2]);
    Scanner scanner(file_contents);
    std::vector<Token> tokens = scanner.scanTokens();
    Parser parser(tokens);
    std::vector<std::shared_ptr<Stmt>> statements = parser.parse();

    if (parser.hadError)
      return 65;

    Interpreter interpreter;
    Resolver resolver(interpreter);
    resolver.resolve(statements);

    // Agar resolver vich koi error aya ta exit karo (If resolver had errors,
    // exit)
    if (resolver.hadError)
      return 65;

    try {
      interpreter.interpret(statements);
    } catch (RuntimeError &error) {
      std::cerr << error.what() << "\n[line " << error.token.line << "]"
                << std::endl;
      return 70;
    }
  } else {
    std::cerr << "Unknown command: " << command << std::endl;
    return 1;
  }

  return 0;
}
