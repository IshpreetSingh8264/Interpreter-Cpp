#ifndef LOX_PARSER_HPP
#define LOX_PARSER_HPP

#include "Expr.hpp"
#include "Stmt.hpp"
#include "Token.hpp"
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "lox.hpp"

namespace lox {

// Parser class: Tokens nu AST (rukh) vich badlan wala
// (Parser class: The guy converting tokens into an AST (Tree))
//
// Layer 2 of the pipeline: Scanner hands it a token vector, it hands the
// Resolver an AST. It never runs the AST and never sees the source text.
class Parser {
public:
  // The book's cap on parameter and argument lists. Exceeding it was a
  // commented-out error call in both grammar rules, so nothing enforced it.
  static constexpr size_t kMaxArity = 255;

  // Error class definition (Standard Lox error exception)
  //
  // A parse error is reported on stderr and then thrown so the grammar rules
  // unwind to the nearest synchronize point in declaration().
  class ParseError : public std::runtime_error {
  public:
    explicit ParseError(const char *msg);
  };

  explicit Parser(const std::vector<Token> &tokens);

  // Main entry: Parse tokens to list of statements
  std::vector<std::shared_ptr<Stmt>> parse();

  // For evaluating expressions only (Codecrafters stage 2/3)
  std::shared_ptr<Expr> parseExpression();

  // Borrowed, not owned: the caller's vector has to outlive the Parser.
  const std::vector<Token> &tokens;
  int current = 0;
  bool hadError = false;

private:
  // Grammar Rules (Niyam)
  // (Grammar Rules: The Laws of the Land)

  // Declarations
  std::shared_ptr<Stmt> declaration();
  std::shared_ptr<Stmt> classDeclaration();
  std::shared_ptr<Stmt> varDeclaration();
  std::shared_ptr<Stmt> function(std::string kind);

  // Statements
  std::shared_ptr<Stmt> statement();
  std::shared_ptr<Stmt> forStatement();
  std::shared_ptr<Stmt> ifStatement();
  std::shared_ptr<Stmt> printStatement();
  std::shared_ptr<Stmt> returnStatement();
  std::shared_ptr<Stmt> whileStatement();
  std::vector<std::shared_ptr<Stmt>> block();
  std::shared_ptr<Stmt> expressionStatement();

  // Expressions
  std::shared_ptr<Expr> expression();
  std::shared_ptr<Expr> assignment();
  std::shared_ptr<Expr> orExpr();
  std::shared_ptr<Expr> andExpr();
  std::shared_ptr<Expr> equality();
  std::shared_ptr<Expr> comparison();
  std::shared_ptr<Expr> term();
  std::shared_ptr<Expr> factor();
  std::shared_ptr<Expr> unary();
  std::shared_ptr<Expr> call();
  std::shared_ptr<Expr> primary();
  std::shared_ptr<Expr> finishCall(std::shared_ptr<Expr> callee);

  // Helpers (Sahayak)
  // (Helpers: The sidekicks)
  bool match(const std::vector<TokenType> &types);
  bool check(TokenType type);
  Token advance();
  bool isAtEnd();
  Token peek();
  Token previous();
  Token consume(TokenType type, std::string message);

  // Error handling
  ParseError error(Token token, std::string message);
  void synchronize();
};

} // namespace lox

#endif // LOX_PARSER_HPP
