#include "vm/isa/instruction_registry.hpp"

#include "vm/isa/handlers/register_handlers.hpp"

namespace vm {

InstructionRegistry::InstructionRegistry() {
    registerDefaultHandlers();
}

ExecutionResult InstructionRegistry::execute(const DecodedInstruction& instruction, ExecutionContext& ctx) {
    const auto it = handlers_.find(instruction.opcode);
    if (it == handlers_.end()) {
        throw VmException("unknown opcode");
    }
    return it->second->execute(instruction, ctx);
}

void InstructionRegistry::registerDefaultHandlers() {
    registerArithmeticHandlers(handlers_);
    registerLogicHandlers(handlers_);
    registerJumpHandlers(handlers_);
    registerMemoryHandlers(handlers_);
    registerStackHandlers(handlers_);
    registerControlHandlers(handlers_);
}

}  // namespace vm
