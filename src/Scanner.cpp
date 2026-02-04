#include <cctype>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "Scanner.hpp"
#include "Token.hpp"
#include "TokenType.hpp"

// Main function: Tokens scan karo!
std::vector<Token> Scanner::scanTokens() {
  // Jeb tak end tak nahi pahunche, scan karde raho
  while (!isAtEnd()) {
    start = current; // Nawa token shuru
    scanToken();
  }

  // Akhir vich EOF token pa do
  tokens.emplace_back(TokenType::END_OF_FILE, "", std::any(), line);
  return tokens;
}

// Helper: Ikalla token scan karo
void Scanner::scanToken() {
  char c = advance();
  switch (c) {
  // Single-character tokens (Ikalle bande)
  case '(':
    addToken(TokenType::LEFT_PAREN);
    break;
  case ')':
    addToken(TokenType::RIGHT_PAREN);
    break;
  case '{':
    addToken(TokenType::LEFT_BRACE);
    break;
  case '}':
    addToken(TokenType::RIGHT_BRACE);
    break;
  case ',':
    addToken(TokenType::COMMA);
    break;
  case '.':
    addToken(TokenType::DOT);
    break;
  case '-':
    addToken(TokenType::MINUS);
    break;
  case '+':
    addToken(TokenType::PLUS);
    break;
  case ';':
    addToken(TokenType::SEMICOLON);
    break;
  case '*':
    addToken(TokenType::STAR);
    break;

  // Two-character tokens (Do bande)
  case '!':
    addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
    break;
  case '=':
    addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
    break;
  case '<':
    addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
    break;
  case '>':
    addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
    break;

  // Slash handling (Comment hai ya Divide?)
  case '/':
    if (match('/')) {
      // Comment labh gya! Line de end tak ignore karo.
      while (peek() != '\n' && !isAtEnd())
        advance();
    } else {
      addToken(TokenType::SLASH);
    }
    break;

  // Whitespace (Vella time/space)
  case ' ':
  case '\r':
  case '\t':
    // Ignore maaro
    break;

  case '\n':
    line++;
    break;

  // String literals (Text da maal)
  case '"':
    string();
    break;

  default:
    if (isdigit(c)) {
      number();
    } else if (isalpha(c) || c == '_') {
      identifier();
    } else {
      // Error! Galt character aa gya.
      // Codecrafters expects error message on stderr
      std::cerr << "[line " << line << "] Error: Unexpected character: " << c
                << std::endl;
      hasError = true; // Error flag set kardo
    }
    break;
  }
}

// Helpers implementation
// Agge wadho
char Scanner::advance() { return source.at(current++); }

// Token add karo list vich
void Scanner::addToken(TokenType type) { addToken(type, std::any()); }

void Scanner::addToken(TokenType type, std::any literal) {
  std::string text = source.substr(start, current - start);
  tokens.emplace_back(type, text, literal, line);
}

// Match checker: Agla character match karda hai?
bool Scanner::match(char expected) {
  if (isAtEnd())
    return false;
  if (source.at(current) != expected)
    return false;

  current++; // Match ho gya, agge wadho
  return true;
}

// Peek helper: Agla character dekho bina agge wadhe
char Scanner::peek() {
  if (isAtEnd())
    return '\0';
  return source.at(current);
}

// Peek Next: Us ton agla character dekho
char Scanner::peekNext() {
  if (current + 1 >= source.length())
    return '\0';
  return source.at(current + 1);
}

// End check: Khatam ho gya?
bool Scanner::isAtEnd() { return current >= source.length(); }

// String scanner
void Scanner::string() {
  // Jab tak closing quote nahi milda
  while (peek() != '"' && !isAtEnd()) {
    if (peek() == '\n')
      line++;
    advance();
  }

  if (isAtEnd()) {
    std::cerr << "[line " << line << "] Error: Unterminated string."
              << std::endl;
    hasError = true;
    return;
  }

  // Closing quote
  advance();

  // Value extract karo (quotes hata ke)
  std::string value = source.substr(start + 1, current - start - 2);
  addToken(TokenType::STRING, value);
}

// Number scanner
void Scanner::number() {
  while (isdigit(peek()))
    advance();

  // Fractional part dekho
  if (peek() == '.' && isdigit(peekNext())) {
    // Dot nu consume karo
    advance();

    // Decimal de baad wale digits
    while (isdigit(peek()))
      advance();
  }

  // Value extract karo te double banao
  std::string numStr = source.substr(start, current - start);
  addToken(TokenType::NUMBER, std::stod(numStr));
}

// Identifier scanner
void Scanner::identifier() {
  while (isalnum(peek()) || peek() == '_')
    advance();

  std::string text = source.substr(start, current - start);
  TokenType type;

  // Check karo keyword hai ya identifier
  auto it = keywords.find(text);
  if (it != keywords.end()) {
    type = it->second;
  } else {
    type = TokenType::IDENTIFIER;
  }

  addToken(type);
}
