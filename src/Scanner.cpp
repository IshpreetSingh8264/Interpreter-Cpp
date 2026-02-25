#include "Scanner.hpp"

#include "Token.hpp"
#include "TokenType.hpp"

#include <cctype>
#include <iostream>
#include <string>
#include <utility>

namespace lox {

Scanner::Scanner(std::string source) : source(std::move(source)) {
  // Reserved keywords load kar lo
  // (Load up the reserved keywords)
  // Ehna da matlab fix hai, change ni ho sakda.
  // (Their meaning is fixed, can't change 'em.)
  keywords["and"] = TokenType::AND;
  keywords["class"] = TokenType::CLASS;
  keywords["else"] = TokenType::ELSE;
  keywords["false"] = TokenType::FALSE;
  keywords["for"] = TokenType::FOR;
  keywords["fun"] = TokenType::FUN;
  keywords["if"] = TokenType::IF;
  keywords["nil"] = TokenType::NIL;
  keywords["or"] = TokenType::OR;
  keywords["print"] = TokenType::PRINT;
  keywords["return"] = TokenType::RETURN;
  keywords["super"] = TokenType::SUPER;
  keywords["this"] = TokenType::THIS;
  keywords["true"] = TokenType::TRUE;
  keywords["var"] = TokenType::VAR;
  keywords["while"] = TokenType::WHILE;
}

// Main function: Tokens scan karo!
// (Main function: Scan those tokens!)
std::vector<Token> Scanner::scanTokens() {
  // Jeb tak end tak nahi pahunche, scan karde raho
  // (Keep scanning until we hit the end of the road)
  while (!isAtEnd()) {
    start = current; // Nawa token shuru
    scanToken();
  }

  // Akhir vich EOF token pa do
  // (Slap an EOF token at the end)
  tokens.emplace_back(TokenType::END_OF_FILE, "", std::any(), line);
  return tokens;
}

// Helper: Ikalla token scan karo
// (Helper: Scan a single token)
void Scanner::scanToken() {
  char c = advance();
  switch (c) {
  // Single-character tokens (Ikalle bande)
  // (Lone wolves)
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
  // (Dynamic duos)
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
  // (Slash handling: Is it a comment or a division? The suspense!)
  case '/':
    if (match('/')) {
      // Comment labh gya! Line de end tak ignore karo.
      // (Found a comment! Ignore everything till the end of the line.)
      while (peek() != '\n' && !isAtEnd())
        advance();
    } else {
      addToken(TokenType::SLASH);
    }
    break;

  // Whitespace (Vella time/space)
  // (Free time/space, just chilling)
  case ' ':
  case '\r':
  case '\t':
    // Ignore maaro
    // (Just ignore it like your ex's texts)
    break;

  case '\n':
    line++;
    break;

  // String literals (Text da maal)
  // (String literals: The textual goods)
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
      // (Error! Wrong character entered the chat.)
      // Codecrafters expects error message on stderr
      std::cerr << "[line " << line << "] Error: Unexpected character: " << c
                << std::endl;
      hasError = true; // Error flag set kardo
                       // (Set the error flag, mission failed successfully)
    }
    break;
  }
}

// Helpers implementation
// Agge wadho
// (Move forward!)
char Scanner::advance() { return source.at(current++); }

// Token add karo list vich
// (Add token to the list)
void Scanner::addToken(TokenType type) { addToken(type, std::any()); }

void Scanner::addToken(TokenType type, std::any literal) {
  std::string text = source.substr(start, current - start);
  tokens.emplace_back(type, text, literal, line);
}

// Match checker: Agla character match karda hai?
// (Match checker: Does the next character match? Consistency is key!)
bool Scanner::match(char expected) {
  if (isAtEnd())
    return false;
  if (source.at(current) != expected)
    return false;

  current++; // Match ho gya, agge wadho
             // (Matched! Move forward!)
  return true;
}

// Peek helper: Agla character dekho bina agge wadhe
// (Peek helper: Look at the next character without moving forward. Sneaky
// peeky!)
char Scanner::peek() {
  if (isAtEnd())
    return '\0';
  return source.at(current);
}

// Peek Next: Us ton agla character dekho
// (Peek Next: Look at the character after the next one. Future vision!)
char Scanner::peekNext() {
  if (static_cast<size_t>(current) + 1 >= source.length())
    return '\0';
  return source.at(current + 1);
}

// End check: Khatam ho gya?
// (End check: Is it over yet?)
bool Scanner::isAtEnd() {
  return static_cast<size_t>(current) >= source.length();
}

// String scanner
void Scanner::string() {
  // Jab tak closing quote nahi milda
  // (Until we find the closing quote)
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
  // (Extract the value, strip the quotes)
  std::string value = source.substr(start + 1, current - start - 2);
  addToken(TokenType::STRING, value);
}

// Number scanner
void Scanner::number() {
  while (isdigit(peek()))
    advance();

  // Fractional part dekho
  // (Check for the fractional part, if any)
  if (peek() == '.' && isdigit(peekNext())) {
    // Dot nu consume karo
    // (Consume the dot, nom nom)
    advance();

    // Decimal de baad wale digits
    // (Digits after the decimal)
    while (isdigit(peek()))
      advance();
  }

  // Value extract karo te double banao
  // (Extract value and make it a double)
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
  // (Check if it's a keyword or just an identifier)
  auto it = keywords.find(text);
  if (it != keywords.end()) {
    type = it->second;
  } else {
    type = TokenType::IDENTIFIER;
  }

  addToken(type);
}

} // namespace lox
