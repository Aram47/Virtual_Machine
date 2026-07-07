#include "vm/assembler/lexer.hpp"

#include <cctype>

namespace vm {

Lexer::Lexer(std::string source) : source_(std::move(source)) {}

void Lexer::skipWhitespace() {
    while (position_ < source_.size()) {
        const char ch = source_[position_];
        if (ch == ' ' || ch == '\t' || ch == '\r') {
            ++position_;
            continue;
        }
        if (ch == '#') {
            while (position_ < source_.size() && source_[position_] != '\n') {
                ++position_;
            }
            continue;
        }
        break;
    }
}

Token Lexer::makeIdentifierOrRegister() {
    const std::size_t start = position_;
    while (position_ < source_.size() &&
           (std::isalnum(static_cast<unsigned char>(source_[position_])) || source_[position_] == '_')) {
        ++position_;
    }
    Token token;
    token.line = line_;
    token.text = source_.substr(start, position_ - start);

    if (!token.text.empty() && token.text[0] == 'r' && token.text.size() > 1) {
        bool digits = true;
        for (char c : token.text.substr(1)) {
            if (!std::isdigit(static_cast<unsigned char>(c))) {
                digits = false;
                break;
            }
        }
        if (digits) {
            token.type = TokenType::Register;
            return token;
        }
    }
    token.type = TokenType::Identifier;
    return token;
}

Token Lexer::makeNumber() {
    const std::size_t start = position_;
    if (position_ < source_.size() && source_[position_] == '-') {
        ++position_;
    }
    while (position_ < source_.size() && std::isdigit(static_cast<unsigned char>(source_[position_]))) {
        ++position_;
    }
    Token token;
    token.type = TokenType::Number;
    token.line = line_;
    token.text = source_.substr(start, position_ - start);
    return token;
}

Token Lexer::readToken() {
    skipWhitespace();
    if (position_ >= source_.size()) {
        return Token{TokenType::End, "", line_};
    }

    const char ch = source_[position_];
    if (ch == '\n') {
        ++position_;
        ++line_;
        return Token{TokenType::Newline, "\\n", line_ - 1};
    }
    if (ch == '[') {
        ++position_;
        return Token{TokenType::LBracket, "[", line_};
    }
    if (ch == ']') {
        ++position_;
        return Token{TokenType::RBracket, "]", line_};
    }
    if (ch == ',') {
        ++position_;
        return Token{TokenType::Comma, ",", line_};
    }
    if (ch == ':') {
        ++position_;
        return Token{TokenType::Colon, ":", line_};
    }
    if (std::isalpha(static_cast<unsigned char>(ch)) || ch == '_') {
        return makeIdentifierOrRegister();
    }
    if (std::isdigit(static_cast<unsigned char>(ch)) || ch == '-') {
        return makeNumber();
    }

    throw AssemblerError(line_, std::string("unexpected character: ") + ch);
}

Token Lexer::next() {
    if (hasPeek_) {
        hasPeek_ = false;
        position_ = peekPosition_;
        line_ = peekLine_;
        return peekToken_;
    }
    return readToken();
}

Token Lexer::peek() {
    if (!hasPeek_) {
        const std::size_t savedPosition = position_;
        const std::size_t savedLine = line_;
        peekToken_ = readToken();
        peekPosition_ = position_;
        peekLine_ = line_;
        position_ = savedPosition;
        line_ = savedLine;
        hasPeek_ = true;
    }
    return peekToken_;
}

void Lexer::reset() {
    position_ = 0;
    line_ = 1;
    hasPeek_ = false;
}

std::size_t Lexer::mark() const {
    return position_;
}

void Lexer::restore(std::size_t mark) {
    position_ = mark;
    hasPeek_ = false;
}

}  // namespace vm
