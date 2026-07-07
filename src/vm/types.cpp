#include "vm/types.hpp"

#include <array>

namespace vm {

Word encodeInstruction(Opcode opcode, std::uint8_t rd, std::uint8_t rs1, std::uint8_t rs2, std::int16_t imm) {
    const Word imm12 = static_cast<Word>(imm) & 0x0FFF;
    const Word encoded =
        (static_cast<Word>(opcode) << 24) |
        (static_cast<Word>(rd & 0x0F) << 20) |
        (static_cast<Word>(rs1 & 0x0F) << 16) |
        (static_cast<Word>(rs2 & 0x0F) << 12) |
        imm12;
    return encoded;
}

DecodedInstruction decodeInstruction(Word raw) {
    DecodedInstruction decoded{};
    decoded.raw = raw;
    decoded.opcode = static_cast<Opcode>((raw >> 24) & 0xFF);
    decoded.rd = static_cast<std::uint8_t>((raw >> 20) & 0x0F);
    decoded.rs1 = static_cast<std::uint8_t>((raw >> 16) & 0x0F);
    decoded.rs2 = static_cast<std::uint8_t>((raw >> 12) & 0x0F);
    std::int32_t imm12 = static_cast<std::int32_t>(raw & 0x0FFF);
    if (imm12 & 0x800) {
        imm12 |= ~0x0FFF;
    }
    decoded.imm = static_cast<std::int16_t>(imm12);
    return decoded;
}

std::uint8_t registerIndexFromName(const std::string& name) {
    if (name.size() < 2 || name[0] != 'r') {
        throw AssemblerError(0, "invalid register name: " + name);
    }
    const int index = std::stoi(name.substr(1));
    if (index < 0 || index >= static_cast<int>(kRegisterCount)) {
        throw AssemblerError(0, "register out of range: " + name);
    }
    return static_cast<std::uint8_t>(index);
}

std::string registerNameFromIndex(std::uint8_t index) {
    return "r" + std::to_string(index);
}

}  // namespace vm
