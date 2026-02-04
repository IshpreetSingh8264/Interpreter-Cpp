#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "Expr.hpp"
#include "Interpreter.hpp"
#include "Parser.hpp"
#include "Resolver.hpp"
#include "RuntimeError.hpp"
#include "Scanner.hpp"
#include "Token.hpp"
#include "TokenType.hpp"

// Utility to read file contents
std::string read_file_contents(const std::string &filename);

// AST Printer (Rukh nu print karo)
// (AST Printer: Print the tree!)
std::string printAst(std::shared_ptr<Expr> expr) {
  if (!expr)
    return "nil";

  if (auto *literal = dynamic_cast<Literal *>(expr.get())) {
    if (!literal->value.has_value())
      return "nil";
    if (literal->value.type() == typeid(bool)) {
      return std::any_cast<bool>(literal->value) ? "true" : "false";
    }
    if (literal->value.type() == typeid(double)) {
      double d = std::any_cast<double>(literal->value);
      std::string s = std::to_string(d);
      // Remove trailing zeros but keep at least one decimal place
      size_t dot = s.find('.');
      if (dot != std::string::npos) {
        size_t last = s.find_last_not_of('0');
        if (last > dot) {
          s = s.substr(0, last + 1);
        } else {
          s = s.substr(0, dot + 2); // Keep one zero after dot
        }
      }
      return s;
    }
    if (literal->value.type() == typeid(std::string)) {
      return std::any_cast<std::string>(literal->value);
    }
    return "nil";
  }

  if (auto *binary = dynamic_cast<Binary *>(expr.get())) {
    return "(" + binary->op.lexeme + " " + printAst(binary->left) + " " +
           printAst(binary->right) + ")";
  }

  if (auto *unary = dynamic_cast<Unary *>(expr.get())) {
    return "(" + unary->op.lexeme + " " + printAst(unary->right) + ")";
  }

  if (auto *grouping = dynamic_cast<Grouping *>(expr.get())) {
    return "(group " + printAst(grouping->expression) + ")";
  }

  if (auto *variable = dynamic_cast<Variable *>(expr.get())) {
    return variable->name.lexeme;
  }

  return "unknown";
}

// Main entry point (Darwaza)
// (Main entry point: The Door)
int main(int argc, char *argv[]) {
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
      std::cout << printAst(expression) << std::endl;
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
