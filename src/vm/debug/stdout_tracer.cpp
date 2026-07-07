#include "vm/debug/stdout_tracer.hpp"

#include "vm/types.hpp"

#include <iostream>

namespace vm {

StdoutTracer::StdoutTracer() : out_(std::cerr) {}

StdoutTracer::StdoutTracer(std::ostream& out) : out_(out) {}

void StdoutTracer::onInstruction(ProcessId pid, Address pc, const DecodedInstruction& insn,
                                 const RegisterFile& regs) {
    out_ << "[pid=" << pid << " pc=" << pc << "] "
         << OpcodeTable::opcodeToMnemonic(insn.opcode) << " | ";
    for (std::uint8_t i = 0; i < kRegisterCount; ++i) {
        out_ << registerNameFromIndex(i) << '=' << regs.get(i) << ' ';
    }
    out_ << "sp=" << regs.sp() << '\n';
}

}  // namespace vm
