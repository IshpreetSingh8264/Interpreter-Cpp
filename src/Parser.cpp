#include "Parser.hpp"
#include "Expr.hpp"
#include "Stmt.hpp"
#include "TokenType.hpp"
#include <iostream>
#include <memory>
#include <vector>

// Main parse function
std::vector<std::shared_ptr<Stmt>> Parser::parse() {
  std::vector<std::shared_ptr<Stmt>> statements;
  while (!isAtEnd()) {
    // Declaration parsing shuru
    statements.push_back(declaration());
  }
  return statements;
}

// Just for testing parsing expressions
std::shared_ptr<Expr> Parser::parseExpression() {
  try {
    return expression();
  } catch (ParseError &error) {
    return nullptr;
  }
}

// --- Declaration Parsing ---

std::shared_ptr<Stmt> Parser::declaration() {
  try {
    if (match({TokenType::VAR}))
      return varDeclaration();
    return statement();
  } catch (ParseError &error) {
    synchronize();
    return nullptr;
  }
}

std::shared_ptr<Stmt> Parser::varDeclaration() {
  Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");

  std::shared_ptr<Expr> initializer = nullptr;
  if (match({TokenType::EQUAL})) {
    initializer = expression();
  }

  consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
  return std::make_shared<Var>(name, initializer);
}

// --- Statement Parsing ---

std::shared_ptr<Stmt> Parser::statement() {
  if (match({TokenType::IF}))
    return ifStatement();
  if (match({TokenType::PRINT}))
    return printStatement();
  if (match({TokenType::WHILE}))
    return whileStatement();
  if (match({TokenType::LEFT_BRACE}))
    return std::make_shared<Block>(block());

  return expressionStatement();
}

std::shared_ptr<Stmt> Parser::ifStatement() {
  consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
  std::shared_ptr<Expr> condition = expression();
  consume(TokenType::RIGHT_PAREN, "Expect ')' after if condition.");

  std::shared_ptr<Stmt> thenBranch = statement();
  std::shared_ptr<Stmt> elseBranch = nullptr;
  if (match({TokenType::ELSE})) {
    elseBranch = statement();
  }

  return std::make_shared<If>(condition, thenBranch, elseBranch);
}

std::shared_ptr<Stmt> Parser::printStatement() {
  std::shared_ptr<Expr> value = expression();
  consume(TokenType::SEMICOLON, "Expect ';' after value.");
  return std::make_shared<Print>(value);
}

std::shared_ptr<Stmt> Parser::whileStatement() {
  consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
  std::shared_ptr<Expr> condition = expression();
  consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");
  std::shared_ptr<Stmt> body = statement();
  return std::make_shared<While>(condition, body);
}

std::vector<std::shared_ptr<Stmt>> Parser::block() {
  std::vector<std::shared_ptr<Stmt>> statements;

  while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
    statements.push_back(declaration());
  }

  consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
  return statements;
}

std::shared_ptr<Stmt> Parser::expressionStatement() {
  std::shared_ptr<Expr> expr = expression();
  consume(TokenType::SEMICOLON, "Expect ';' after expression.");
  return std::make_shared<Expression>(expr);
}

// --- Expression Parsing ---

std::shared_ptr<Expr> Parser::expression() { return assignment(); }

std::shared_ptr<Expr> Parser::assignment() {
  std::shared_ptr<Expr> expr = orExpr();

  if (match({TokenType::EQUAL})) {
    Token equals = previous();
    std::shared_ptr<Expr> value = assignment();

    // Check if target is a variable
    if (auto varExpr = std::dynamic_pointer_cast<Variable>(expr)) {
      Token name = varExpr->name;
      return std::make_shared<Assign>(name, value);
    }

    error(equals, "Invalid assignment target.");
  }

  return expr;
}

std::shared_ptr<Expr> Parser::orExpr() {
  std::shared_ptr<Expr> expr = andExpr();

  while (match({TokenType::OR})) {
    Token op = previous();
    std::shared_ptr<Expr> right = andExpr();
    expr = std::make_shared<Logical>(expr, op, right);
  }

  return expr;
}

std::shared_ptr<Expr> Parser::andExpr() {
  std::shared_ptr<Expr> expr = equality();

  while (match({TokenType::AND})) {
    Token op = previous();
    std::shared_ptr<Expr> right = equality();
    expr = std::make_shared<Logical>(expr, op, right);
  }

  return expr;
}

std::shared_ptr<Expr> Parser::equality() {
  std::shared_ptr<Expr> expr = comparison();

  while (match({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL})) {
    Token op = previous();
    std::shared_ptr<Expr> right = comparison();
    expr = std::make_shared<Binary>(expr, op, right);
  }

  return expr;
}

std::shared_ptr<Expr> Parser::comparison() {
  std::shared_ptr<Expr> expr = term();

  while (match({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS,
                TokenType::LESS_EQUAL})) {
    Token op = previous();
    std::shared_ptr<Expr> right = term();
    expr = std::make_shared<Binary>(expr, op, right);
  }

  return expr;
}

std::shared_ptr<Expr> Parser::term() {
  std::shared_ptr<Expr> expr = factor();

  while (match({TokenType::MINUS, TokenType::PLUS})) {
    Token op = previous();
    std::shared_ptr<Expr> right = factor();
    expr = std::make_shared<Binary>(expr, op, right);
  }

  return expr;
}

std::shared_ptr<Expr> Parser::factor() {
  std::shared_ptr<Expr> expr = unary();

  while (match({TokenType::SLASH, TokenType::STAR})) {
    Token op = previous();
    std::shared_ptr<Expr> right = unary();
    expr = std::make_shared<Binary>(expr, op, right);
  }

  return expr;
}

std::shared_ptr<Expr> Parser::unary() {
  if (match({TokenType::BANG, TokenType::MINUS})) {
    Token op = previous();
    std::shared_ptr<Expr> right = unary();
    return std::make_shared<Unary>(op, right);
  }

  return call();
}

std::shared_ptr<Expr> Parser::call() {
  std::shared_ptr<Expr> expr = primary();

  while (true) {
    if (match({TokenType::LEFT_PAREN})) {
      expr = finishCall(expr);
    } else {
      break;
    }
  }

  return expr;
}

std::shared_ptr<Expr> Parser::finishCall(std::shared_ptr<Expr> callee) {
  std::vector<std::shared_ptr<Expr>> arguments;
  if (!check(TokenType::RIGHT_PAREN)) {
    do {
      if (arguments.size() >= 255) {
        // error(peek(), "Can't have more than 255 arguments.");
        // For simplicity just ignore or print
      }
      arguments.push_back(expression());
    } while (match({TokenType::COMMA}));
  }

  Token paren = consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
  return std::make_shared<Call>(callee, paren, arguments);
}

std::shared_ptr<Expr> Parser::primary() {
  if (match({TokenType::FALSE}))
    return std::make_shared<Literal>(false);
  if (match({TokenType::TRUE}))
    return std::make_shared<Literal>(true);
  if (match({TokenType::NIL}))
    return std::make_shared<Literal>(std::any()); // null

  if (match({TokenType::NUMBER, TokenType::STRING})) {
    return std::make_shared<Literal>(previous().literal);
  }

  if (match({TokenType::IDENTIFIER})) {
    return std::make_shared<Variable>(previous());
  }

  if (match({TokenType::LEFT_PAREN})) {
    std::shared_ptr<Expr> expr = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
    return std::make_shared<Grouping>(expr);
  }

  throw error(peek(), "Expect expression.");
}

// --- Helpers ---

bool Parser::match(const std::vector<TokenType> &types) {
  for (TokenType type : types) {
    if (check(type)) {
      advance();
      return true;
    }
  }
  return false;
}

bool Parser::check(TokenType type) {
  if (isAtEnd())
    return false;
  return peek().type == type;
}

Token Parser::advance() {
  if (!isAtEnd())
    current++;
  return previous();
}

bool Parser::isAtEnd() { return peek().type == TokenType::END_OF_FILE; }

Token Parser::peek() { return tokens.at(current); }

Token Parser::previous() { return tokens.at(current - 1); }

Token Parser::consume(TokenType type, std::string message) {
  if (check(type))
    return advance();
  throw error(peek(), message);
}

Parser::ParseError Parser::error(Token token, std::string message) {
  // Report error but don't crash program yet, just sync
  // Codecrafters expects output to stderr
  // if (token.type == TokenType::END_OF_FILE) {
  //     std::cerr << "[line " << token.line << "] Error at end: " << message <<
  //     std::endl;
  // } else {
  //     std::cerr << "[line " << token.line << "] Error at '" << token.lexeme
  //     << "': " << message << std::endl;
  // }
  // Using standard format for now
  std::cerr << "[line " << token.line << "] Error";
  if (token.type == TokenType::END_OF_FILE) {
    std::cerr << " at end";
  } else {
    std::cerr << " at '" << token.lexeme << "'";
  }
  std::cerr << ": " << message << std::endl;

  hadError = true;
  return ParseError(message.c_str());
}

void Parser::synchronize() {
  advance();

  while (!isAtEnd()) {
    if (previous().type == TokenType::SEMICOLON)
      return;

    switch (peek().type) {
    case TokenType::CLASS:
    case TokenType::FUN:
    case TokenType::VAR:
    case TokenType::FOR:
    case TokenType::IF:
    case TokenType::WHILE:
    case TokenType::PRINT:
    case TokenType::RETURN:
      return;
    default:
      // Keep advancing
      break;
    }

    advance();
  }
}
