#include "vm/isa/opcode_table.hpp"

#include <unordered_map>

namespace vm {

Opcode OpcodeTable::mnemonicToOpcode(const std::string& name) {
    static const std::unordered_map<std::string, Opcode> mapping = {
        {"mov", Opcode::MOV},   {"add", Opcode::ADD},   {"sub", Opcode::SUB},
        {"mul", Opcode::MUL},   {"div", Opcode::DIV},   {"mod", Opcode::MOD},
        {"inc", Opcode::INC},   {"dec", Opcode::DEC},   {"and", Opcode::AND},
        {"or", Opcode::OR},     {"xor", Opcode::XOR},   {"not", Opcode::NOT},
        {"cmp", Opcode::CMP},   {"jmp", Opcode::JMP},   {"je", Opcode::JE},
        {"jne", Opcode::JNE},   {"jg", Opcode::JG},     {"jge", Opcode::JGE},
        {"jl", Opcode::JL},     {"jle", Opcode::JLE},   {"load", Opcode::LOAD},
        {"store", Opcode::STORE}, {"push", Opcode::PUSH}, {"pop", Opcode::POP},
        {"ret", Opcode::RET},   {"halt", Opcode::HALT}, {"nop", Opcode::NOP},
    };

    const auto it = mapping.find(name);
    if (it == mapping.end()) {
        throw AssemblerError(0, "unknown opcode: " + name);
    }
    return it->second;
}

std::string OpcodeTable::opcodeToMnemonic(Opcode opcode) {
    switch (opcode) {
        case Opcode::MOV:
            return "mov";
        case Opcode::ADD:
            return "add";
        case Opcode::SUB:
            return "sub";
        case Opcode::MUL:
            return "mul";
        case Opcode::DIV:
            return "div";
        case Opcode::MOD:
            return "mod";
        case Opcode::INC:
            return "inc";
        case Opcode::DEC:
            return "dec";
        case Opcode::AND:
            return "and";
        case Opcode::OR:
            return "or";
        case Opcode::XOR:
            return "xor";
        case Opcode::NOT:
            return "not";
        case Opcode::CMP:
            return "cmp";
        case Opcode::JMP:
            return "jmp";
        case Opcode::JE:
            return "je";
        case Opcode::JNE:
            return "jne";
        case Opcode::JG:
            return "jg";
        case Opcode::JGE:
            return "jge";
        case Opcode::JL:
            return "jl";
        case Opcode::JLE:
            return "jle";
        case Opcode::LOAD:
            return "load";
        case Opcode::STORE:
            return "store";
        case Opcode::CALL:
            return "call";
        case Opcode::HALT:
            return "halt";
        case Opcode::NOP:
            return "nop";
        case Opcode::PUSH:
            return "push";
        case Opcode::POP:
            return "pop";
        case Opcode::RET:
            return "ret";
    }
    return "???";
}

}  // namespace vm
