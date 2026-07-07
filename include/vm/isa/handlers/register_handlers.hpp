#pragma once

#include "vm/isa/i_instruction_handler.hpp"

#include <memory>
#include <unordered_map>

namespace vm {

void registerArithmeticHandlers(
    std::unordered_map<Opcode, std::unique_ptr<IInstructionHandler>>& handlers);
void registerLogicHandlers(
    std::unordered_map<Opcode, std::unique_ptr<IInstructionHandler>>& handlers);
void registerJumpHandlers(
    std::unordered_map<Opcode, std::unique_ptr<IInstructionHandler>>& handlers);
void registerMemoryHandlers(
    std::unordered_map<Opcode, std::unique_ptr<IInstructionHandler>>& handlers);
void registerStackHandlers(
    std::unordered_map<Opcode, std::unique_ptr<IInstructionHandler>>& handlers);
void registerControlHandlers(
    std::unordered_map<Opcode, std::unique_ptr<IInstructionHandler>>& handlers);

}  // namespace vm
