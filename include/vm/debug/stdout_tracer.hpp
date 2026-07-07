#pragma once

#include "vm/debug/i_execution_tracer.hpp"
#include "vm/isa/opcode_table.hpp"

#include <iostream>
#include <ostream>

namespace vm {

class StdoutTracer : public IExecutionTracer {
public:
    StdoutTracer();
    explicit StdoutTracer(std::ostream& out);

    void onInstruction(ProcessId pid, Address pc, const DecodedInstruction& insn,
                       const RegisterFile& regs) override;

private:
    std::ostream& out_;
};

}  // namespace vm
