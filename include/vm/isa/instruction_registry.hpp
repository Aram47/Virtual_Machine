#pragma once

#include "vm/isa/i_instruction_handler.hpp"
#include "vm/isa/instruction.hpp"

#include <memory>
#include <unordered_map>

namespace vm {

class InstructionRegistry {
public:
    InstructionRegistry();

    ExecutionResult execute(const DecodedInstruction& instruction, ExecutionContext& ctx);

private:
    std::unordered_map<Opcode, std::unique_ptr<IInstructionHandler>> handlers_;

    void registerDefaultHandlers();
};

}  // namespace vm
