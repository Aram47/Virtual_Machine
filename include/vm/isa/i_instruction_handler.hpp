#pragma once

#include "vm/isa/instruction.hpp"

namespace vm {

class IInstructionHandler {
public:
    virtual ~IInstructionHandler() = default;
    virtual ExecutionResult execute(const DecodedInstruction& instruction,
                                    ExecutionContext& ctx) = 0;
};

}  // namespace vm
