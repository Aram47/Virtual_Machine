#pragma once

#include "vm/types.hpp"

#include <string>
#include <vector>

namespace vm {

enum class TokenType {
    End,
    Newline,
    Identifier,
    Number,
    Register,
    LBracket,
    RBracket,
    Comma,
    Colon,
};

struct Token {
    TokenType type = TokenType::End;
    std::string text;
    std::size_t line = 1;
};

class Lexer {
public:
    explicit Lexer(std::string source);

    Token next();
    Token peek();
    void reset();

    std::size_t mark() const;
    void restore(std::size_t mark);

private:
    void skipWhitespace();
    Token readToken();
    Token makeIdentifierOrRegister();
    Token makeNumber();

    std::string source_;
    std::size_t position_ = 0;
    std::size_t line_ = 1;
    bool hasPeek_ = false;
    Token peekToken_;
    std::size_t peekPosition_ = 0;
    std::size_t peekLine_ = 1;
};

}  // namespace vm
