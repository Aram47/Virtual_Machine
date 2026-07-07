#include "vm/isa/instruction_helpers.hpp"

#include "vm/process/process_control_block.hpp"

namespace vm {

Word readOperand(const DecodedInstruction& instruction, ExecutionContext& ctx) {
    if (instruction.rs2 == kImmediateMarker) {
        return static_cast<Word>(static_cast<std::int32_t>(instruction.imm));
    }
    return ctx.cpu.registers().get(instruction.rs2);
}

bool takeJump(const CpuFlags& flags, Opcode opcode) {
    switch (opcode) {
        case Opcode::JE:
            return flags.zero;
        case Opcode::JNE:
            return !flags.zero;
        case Opcode::JG:
            return !flags.zero && !flags.sign;
        case Opcode::JGE:
            return !flags.sign;
        case Opcode::JL:
            return flags.sign;
        case Opcode::JLE:
            return flags.zero || flags.sign;
        default:
            return false;
    }
}

void setExecutionError(ExecutionContext& ctx, const char* message) {
    if (ctx.process) {
        ctx.process->setLastError(message);
    }
}

}  // namespace vm
