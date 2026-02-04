#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "Expr.hpp"
#include "Interpreter.hpp"
#include "Parser.hpp"
#include "RuntimeError.hpp"
#include "Scanner.hpp"
#include "Token.hpp"
#include "TokenType.hpp"

// Utility to read file contents
std::string read_file_contents(const std::string &filename);

// Main entry point (Darwaza)
int main(int argc, char *argv[]) {
  // Disable output buffering
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  if (argc < 3) {
    std::cerr << "Usage: ./your_program <command> <filename>" << std::endl;
    return 1;
  }

  const std::string command = argv[1];

  if (command == "tokenize") {
    std::string file_contents = read_file_contents(argv[2]);
    Scanner scanner(file_contents);
    std::vector<Token> tokens = scanner.scanTokens();
    for (const Token &token : tokens) {
      std::cout << token.toString() << std::endl;
    }
    if (scanner.hasError)
      return 65;
  } else if (command == "parse") {
    std::string file_contents = read_file_contents(argv[2]);
    Scanner scanner(file_contents);
    std::vector<Token> tokens = scanner.scanTokens();
    Parser parser(tokens);
    std::vector<std::shared_ptr<Stmt>> statements = parser.parse();
    if (parser.hadError)
      return 65;
    // Parsing done (Checking syntax errors)
    // If we wanted to print AST, we would need AstPrinter.
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
        std::cout << interpreter.stringify(result) << std::endl;
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
