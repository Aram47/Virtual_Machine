#pragma once

#include "vm/assembler/lexer.hpp"

#include <unordered_map>
#include <vector>

namespace vm {

enum class DataType {
    Byte,
    DByte,
    Word,
    QWord,
};

struct DataSymbol {
    std::string name;
    DataType type;
    std::int64_t value;
    Address offset;
};

struct AssembledProgram {
    std::vector<Word> code;
    std::vector<std::byte> data;
};

class Parser {
public:
    AssembledProgram parse(const std::string& source);

private:
    enum class Section { None, Data, Code };

    void firstPass(Lexer& lexer);
    void secondPass(Lexer& lexer, AssembledProgram& program);

    Token consume(Lexer& lexer, TokenType type);
    bool match(Lexer& lexer, TokenType type);
    void skipNewlines(Lexer& lexer);
    Token current(Lexer& lexer);

    void parseProgram(Lexer& lexer, bool emit, std::vector<Word>& code);
    void parseInstruction(Lexer& lexer, bool emit, std::vector<Word>& code);

    Word encodeMov(std::uint8_t rd, std::uint8_t rsOrImmMarker, std::int16_t imm);
    Word encodeRegReg(Opcode opcode, std::uint8_t rd, std::uint8_t rs1, std::uint8_t rs2);
    Word encodeRegImm(Opcode opcode, std::uint8_t rd, std::uint8_t rs1, std::int16_t imm);
    Word encodeJump(Opcode opcode, Address target);
    Word encodeCall(SyscallId syscall, Address arg);

    std::uint8_t parseRegisterToken(const Token& token);
    std::int16_t parseNumberToken(const Token& token);
    Address resolveLabel(const std::string& label, std::size_t line) const;
  Address resolveDataSymbol(const std::string& name, std::size_t line) const;
    Opcode parseOpcode(const std::string& name) const;
    SyscallId parseSyscall(const std::string& name) const;
    std::size_t dataTypeSize(DataType type) const;

    Section section_ = Section::None;
    Address codeAddress_ = 0;
    Address dataOffset_ = 0;
    std::unordered_map<std::string, Address> labels_;
    std::vector<DataSymbol> dataSymbols_;
};

}  // namespace vm
