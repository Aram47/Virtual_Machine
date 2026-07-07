#include "vm/assembler/parser.hpp"

#include "vm/isa/encode.hpp"
#include "vm/isa/opcode_table.hpp"

#include <algorithm>

namespace vm {

AssembledProgram Parser::parse(const std::string& source) {
    labels_.clear();
    dataSymbols_.clear();
    section_ = Section::None;
    codeAddress_ = 0;
    dataOffset_ = 0;

    Lexer lexer(source);
    firstPass(lexer);

    lexer.reset();
    section_ = Section::None;
    codeAddress_ = 0;

    AssembledProgram program;
    secondPass(lexer, program);
    return program;
}

void Parser::firstPass(Lexer& lexer) {
    std::vector<Word> dummy;
    parseProgram(lexer, false, dummy);
}

void Parser::secondPass(Lexer& lexer, AssembledProgram& program) {
    codeAddress_ = 0;
    program.data.resize(dataOffset_, std::byte{0});
    for (const auto& symbol : dataSymbols_) {
        switch (symbol.type) {
            case DataType::Byte:
                program.data[symbol.offset] = static_cast<std::byte>(symbol.value & 0xFF);
                break;
            case DataType::DByte: {
                const auto value = static_cast<std::uint16_t>(symbol.value);
                program.data[symbol.offset] = static_cast<std::byte>(value & 0xFF);
                program.data[symbol.offset + 1] = static_cast<std::byte>((value >> 8) & 0xFF);
                break;
            }
            case DataType::Word: {
                const auto value = static_cast<std::uint32_t>(symbol.value);
                for (std::size_t i = 0; i < 4; ++i) {
                    program.data[symbol.offset + i] =
                        static_cast<std::byte>((value >> (8 * i)) & 0xFF);
                }
                break;
            }
            case DataType::QWord: {
                std::uint64_t value = static_cast<std::uint64_t>(symbol.value);
                for (std::size_t i = 0; i < 8; ++i) {
                    program.data[symbol.offset + i] =
                        static_cast<std::byte>((value >> (8 * i)) & 0xFF);
                }
                break;
            }
        }
    }

    parseProgram(lexer, true, program.code);
}

void Parser::parseProgram(Lexer& lexer, bool emit, std::vector<Word>& code) {
    section_ = Section::None;
    skipNewlines(lexer);
    while (lexer.peek().type != TokenType::End) {
        skipNewlines(lexer);
        if (lexer.peek().type == TokenType::End) {
            break;
        }

        const std::size_t statementStart = lexer.mark();
        Token token = lexer.next();

        if (token.type == TokenType::Identifier && token.text == "DATA") {
            consume(lexer, TokenType::Colon);
            section_ = Section::Data;
            skipNewlines(lexer);
            continue;
        }
        if (token.type == TokenType::Identifier && token.text == "CODE") {
            consume(lexer, TokenType::Colon);
            section_ = Section::Code;
            skipNewlines(lexer);
            continue;
        }

        if (section_ == Section::Data) {
            if (token.type != TokenType::Identifier) {
                throw AssemblerError(token.line, "expected data type");
            }
            DataType dataType;
            if (token.text == "BYTE") {
                dataType = DataType::Byte;
            } else if (token.text == "DBYTE") {
                dataType = DataType::DByte;
            } else if (token.text == "WORD") {
                dataType = DataType::Word;
            } else if (token.text == "QWORD") {
                dataType = DataType::QWord;
            } else {
                throw AssemblerError(token.line, "unknown data type: " + token.text);
            }

            const Token nameToken = consume(lexer, TokenType::Identifier);
            const Token valueToken = consume(lexer, TokenType::Number);
            const auto value = static_cast<std::int64_t>(parseNumberToken(valueToken));

            if (!emit) {
                DataSymbol symbol{nameToken.text, dataType, value, dataOffset_};
                dataSymbols_.push_back(symbol);
                dataOffset_ += dataTypeSize(dataType);
            }
            skipNewlines(lexer);
            continue;
        }

        if (section_ == Section::Code) {
            if (token.type == TokenType::Identifier && lexer.peek().type == TokenType::Colon) {
                lexer.next();
                labels_[token.text] = codeAddress_;
                skipNewlines(lexer);
                continue;
            }

            lexer.restore(statementStart);
            parseInstruction(lexer, emit, code);
            continue;
        }

        throw AssemblerError(token.line, "expected DATA or CODE section");
    }
}

Token Parser::consume(Lexer& lexer, TokenType type) {
    Token token = lexer.next();
    if (token.type != type) {
        throw AssemblerError(token.line, "unexpected token: " + token.text);
    }
    return token;
}

bool Parser::match(Lexer& lexer, TokenType type) {
    if (lexer.peek().type == type) {
        lexer.next();
        return true;
    }
    return false;
}

void Parser::skipNewlines(Lexer& lexer) {
    while (match(lexer, TokenType::Newline)) {
    }
}

Token Parser::current(Lexer& lexer) {
    return lexer.peek();
}

void Parser::parseInstruction(Lexer& lexer, bool emit, std::vector<Word>& code) {
    const Token opcodeToken = consume(lexer, TokenType::Identifier);
    const std::string opcodeName = opcodeToken.text;

    if (opcodeName == "halt") {
        if (emit) {
            code.push_back(encodeInstruction(Opcode::HALT, 0, 0, 0, 0));
        }
        ++codeAddress_;
        skipNewlines(lexer);
        return;
    }

    if (opcodeName == "ret") {
        if (emit) {
            code.push_back(encodeInstruction(Opcode::RET, 0, 0, 0, 0));
        }
        ++codeAddress_;
        skipNewlines(lexer);
        return;
    }

    if (opcodeName == "push") {
        if (lexer.peek().type == TokenType::Register) {
            const Token regToken = consume(lexer, TokenType::Register);
            const std::uint8_t rd = parseRegisterToken(regToken);
            if (emit) {
                code.push_back(encodeRegReg(Opcode::PUSH, rd, rd, rd));
            }
        } else if (current(lexer).type == TokenType::Identifier) {
            const Token labelToken = consume(lexer, TokenType::Identifier);
            if (emit) {
                const Address target = resolveLabel(labelToken.text, labelToken.line);
                code.push_back(encodeRegImm(Opcode::PUSH, 0, 0, static_cast<std::int16_t>(target)));
            }
        } else {
            const Token numberToken = consume(lexer, TokenType::Number);
            const std::int16_t imm = parseNumberToken(numberToken);
            if (emit) {
                code.push_back(encodeRegImm(Opcode::PUSH, 0, 0, imm));
            }
        }
        ++codeAddress_;
        skipNewlines(lexer);
        return;
    }

    if (opcodeName == "pop") {
        const std::uint8_t rd = parseRegisterToken(consume(lexer, TokenType::Register));
        if (emit) {
            code.push_back(encodeInstruction(Opcode::POP, rd, 0, 0, 0));
        }
        ++codeAddress_;
        skipNewlines(lexer);
        return;
    }

    if (opcodeName == "call") {
        const Token syscallToken = consume(lexer, TokenType::Identifier);
        consume(lexer, TokenType::Comma);
        Address arg = 0;
        if (match(lexer, TokenType::LBracket)) {
            const Token argToken = consume(lexer, TokenType::Identifier);
            arg = resolveDataSymbol(argToken.text, argToken.line);
            consume(lexer, TokenType::RBracket);
        } else {
            const Token argToken = consume(lexer, TokenType::Number);
            arg = static_cast<Address>(parseNumberToken(argToken));
        }
        if (emit) {
            code.push_back(encodeCall(parseSyscall(syscallToken.text), arg));
        }
        ++codeAddress_;
        skipNewlines(lexer);
        return;
    }

    const Opcode opcode = parseOpcode(opcodeName);

    if (opcode == Opcode::JMP || opcode == Opcode::JE || opcode == Opcode::JNE || opcode == Opcode::JG ||
        opcode == Opcode::JGE || opcode == Opcode::JL || opcode == Opcode::JLE) {
        const Token labelToken = consume(lexer, TokenType::Identifier);
        if (emit) {
            code.push_back(encodeJump(opcode, resolveLabel(labelToken.text, labelToken.line)));
        }
        ++codeAddress_;
        skipNewlines(lexer);
        return;
    }

    if (opcode == Opcode::INC || opcode == Opcode::DEC || opcode == Opcode::NOT) {
        const std::uint8_t rd = parseRegisterToken(consume(lexer, TokenType::Register));
        if (emit) {
            code.push_back(encodeInstruction(opcode, rd, 0, 0, 0));
        }
        ++codeAddress_;
        skipNewlines(lexer);
        return;
    }

    if (opcode == Opcode::LOAD) {
        const std::uint8_t rd = parseRegisterToken(consume(lexer, TokenType::Register));
        consume(lexer, TokenType::Comma);
        Address offset = 0;
        if (match(lexer, TokenType::LBracket)) {
            const Token symToken = consume(lexer, TokenType::Identifier);
            offset = resolveDataSymbol(symToken.text, symToken.line);
            consume(lexer, TokenType::RBracket);
        } else {
            const Token numToken = consume(lexer, TokenType::Number);
            offset = static_cast<Address>(parseNumberToken(numToken));
        }
        if (emit) {
            code.push_back(encodeInstruction(Opcode::LOAD, rd, 0, 0, static_cast<std::int16_t>(offset)));
        }
        ++codeAddress_;
        skipNewlines(lexer);
        return;
    }

    if (opcode == Opcode::STORE) {
        const std::uint8_t rd = parseRegisterToken(consume(lexer, TokenType::Register));
        consume(lexer, TokenType::Comma);
        Address offset = 0;
        if (match(lexer, TokenType::LBracket)) {
            const Token symToken = consume(lexer, TokenType::Identifier);
            offset = resolveDataSymbol(symToken.text, symToken.line);
            consume(lexer, TokenType::RBracket);
        } else {
            const Token numToken = consume(lexer, TokenType::Number);
            offset = static_cast<Address>(parseNumberToken(numToken));
        }
        if (emit) {
            code.push_back(encodeInstruction(Opcode::STORE, rd, 0, 0, static_cast<std::int16_t>(offset)));
        }
        ++codeAddress_;
        skipNewlines(lexer);
        return;
    }

    const std::uint8_t rd = parseRegisterToken(consume(lexer, TokenType::Register));
    consume(lexer, TokenType::Comma);

    if (lexer.peek().type == TokenType::Register) {
        const std::uint8_t rs = parseRegisterToken(consume(lexer, TokenType::Register));
        if (emit) {
            if (opcode == Opcode::MOV) {
                code.push_back(encodeRegReg(opcode, rd, rs, rs));
            } else {
                code.push_back(encodeRegReg(opcode, rd, rd, rs));
            }
        }
    } else if (match(lexer, TokenType::LBracket)) {
        const Token symToken = consume(lexer, TokenType::Identifier);
        const Address offset = resolveDataSymbol(symToken.text, symToken.line);
        consume(lexer, TokenType::RBracket);
        if (emit) {
            if (opcode == Opcode::MOV) {
                code.push_back(encodeInstruction(Opcode::LOAD, rd, 0, 0, static_cast<std::int16_t>(offset)));
            } else {
                throw AssemblerError(symToken.line, "memory operand not supported for this opcode");
            }
        }
    } else {
        const Token immToken = consume(lexer, TokenType::Number);
        const std::int16_t imm = parseNumberToken(immToken);
        if (emit) {
            if (opcode == Opcode::MOV) {
                code.push_back(encodeMov(rd, kImmediateMarker, imm));
            } else if (opcode == Opcode::CMP) {
                code.push_back(encodeRegImm(opcode, 0, rd, imm));
            } else {
                code.push_back(encodeRegImm(opcode, rd, rd, imm));
            }
        }
    }

    ++codeAddress_;
    skipNewlines(lexer);
}

Word Parser::encodeMov(std::uint8_t rd, std::uint8_t rsOrImmMarker, std::int16_t imm) {
    return encodeInstruction(Opcode::MOV, rd, 0, rsOrImmMarker, imm);
}

Word Parser::encodeRegReg(Opcode opcode, std::uint8_t rd, std::uint8_t rs1, std::uint8_t rs2) {
    return encodeInstruction(opcode, rd, rs1, rs2, 0);
}

Word Parser::encodeRegImm(Opcode opcode, std::uint8_t rd, std::uint8_t rs1, std::int16_t imm) {
    return encodeInstruction(opcode, rd, rs1, kImmediateMarker, imm);
}

Word Parser::encodeJump(Opcode opcode, Address target) {
    return encodeInstruction(opcode, 0, 0, 0, static_cast<std::int16_t>(target));
}

Word Parser::encodeCall(SyscallId syscall, Address arg) {
    return encodeInstruction(Opcode::CALL, 0, static_cast<std::uint8_t>(syscall), 0,
                             static_cast<std::int16_t>(arg));
}

std::uint8_t Parser::parseRegisterToken(const Token& token) {
    if (token.type != TokenType::Register) {
        throw AssemblerError(token.line, "expected register");
    }
    return registerIndexFromName(token.text);
}

std::int16_t Parser::parseNumberToken(const Token& token) {
    if (token.type != TokenType::Number) {
        throw AssemblerError(token.line, "expected number");
    }
    const long value = std::stol(token.text);
    return static_cast<std::int16_t>(value);
}

Address Parser::resolveLabel(const std::string& label, std::size_t line) const {
    const auto it = labels_.find(label);
    if (it == labels_.end()) {
        throw AssemblerError(line, "unknown label: " + label);
    }
    return it->second;
}

Address Parser::resolveDataSymbol(const std::string& name, std::size_t line) const {
    for (const auto& symbol : dataSymbols_) {
        if (symbol.name == name) {
            return symbol.offset;
        }
    }
    throw AssemblerError(line, "unknown data symbol: " + name);
}

Opcode Parser::parseOpcode(const std::string& name) const {
    return OpcodeTable::mnemonicToOpcode(name);
}

SyscallId Parser::parseSyscall(const std::string& name) const {
    if (name == "print") {
        return SyscallId::Print;
    }
    if (name == "scan") {
        return SyscallId::Scan;
    }
    if (name == "exit") {
        return SyscallId::Exit;
    }
    if (name == "yield") {
        return SyscallId::Yield;
    }
    if (name == "load") {
        return SyscallId::Load;
    }
    throw AssemblerError(0, "unknown syscall: " + name);
}

std::size_t Parser::dataTypeSize(DataType type) const {
    switch (type) {
        case DataType::Byte:
            return 1;
        case DataType::DByte:
            return 2;
        case DataType::Word:
            return 4;
        case DataType::QWord:
            return 8;
    }
    return 1;
}

}  // namespace vm
