#pragma once

#include "vm/isa/encode.hpp"
#include "vm/isa/instruction.hpp"

namespace vm {

Word readOperand(const DecodedInstruction& instruction, ExecutionContext& ctx);
bool takeJump(const CpuFlags& flags, Opcode opcode);
void setExecutionError(ExecutionContext& ctx, const char* message);

}  // namespace vm
