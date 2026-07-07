#pragma once

#include "vm/cpu/register_file.hpp"
#include "vm/types.hpp"

namespace vm {

class IExecutionTracer {
public:
    virtual ~IExecutionTracer() = default;
    virtual void onInstruction(ProcessId pid, Address pc, const DecodedInstruction& insn,
                               const RegisterFile& regs) = 0;
};

}  // namespace vm
