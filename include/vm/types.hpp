#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <stdexcept>

namespace vm {

using Word = std::uint32_t;
using Address = std::uint32_t;
using ProcessId = std::uint32_t;

constexpr std::size_t kRegisterCount = 16;
constexpr std::size_t kDefaultProcessMemorySize = 64 * 1024;
constexpr std::size_t kDefaultPhysicalMemorySize = 1024 * 1024;
constexpr std::size_t kDefaultTimeSlice = 100;
constexpr std::size_t kDefaultCoreCount = 4;
constexpr std::size_t kStackSize = 4096;

enum class Opcode : std::uint8_t {
    MOV = 0,
    ADD = 1,
    SUB = 2,
    MUL = 3,
    DIV = 4,
    MOD = 5,
    INC = 6,
    DEC = 7,
    AND = 8,
    OR = 9,
    XOR = 10,
    NOT = 11,
    CMP = 12,
    JMP = 13,
    JE = 14,
    JNE = 15,
    JG = 16,
    JGE = 17,
    JL = 18,
    JLE = 19,
    LOAD = 20,
    STORE = 21,
    CALL = 22,
    HALT = 23,
    NOP = 24,
    PUSH = 25,
    POP = 26,
    RET = 27,
};

enum class SyscallId : std::uint16_t {
    Print = 0,
    Scan = 1,
    Exit = 2,
    Yield = 3,
    Load = 4,
};

struct CpuFlags {
    bool zero = false;
    bool sign = false;
    bool carry = false;
    bool overflow = false;

    void clear() {
        zero = sign = carry = overflow = false;
    }
};

struct DecodedInstruction {
    Opcode opcode = Opcode::NOP;
    std::uint8_t rd = 0;
    std::uint8_t rs1 = 0;
    std::uint8_t rs2 = 0;
    std::int16_t imm = 0;
    Word raw = 0;
};

class VmException : public std::runtime_error {
public:
    explicit VmException(const std::string& message) : std::runtime_error(message) {}
};

class AssemblerError : public std::runtime_error {
public:
  AssemblerError(std::size_t line, const std::string& message)
      : std::runtime_error("line " + std::to_string(line) + ": " + message), line_(line) {}

    std::size_t line() const { return line_; }

private:
    std::size_t line_;
};

Word encodeInstruction(Opcode opcode, std::uint8_t rd, std::uint8_t rs1, std::uint8_t rs2, std::int16_t imm);
DecodedInstruction decodeInstruction(Word raw);

std::uint8_t registerIndexFromName(const std::string& name);
std::string registerNameFromIndex(std::uint8_t index);

}  // namespace vm
