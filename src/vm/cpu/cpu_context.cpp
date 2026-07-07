#include "vm/cpu/cpu_context.hpp"

namespace vm {

CpuContext::CpuContext() = default;

void CpuContext::saveTo(CpuContext& other) const {
    other.registers_.setGpr(registers_.gpr());
    other.registers_.setPc(registers_.pc());
    other.registers_.setSp(registers_.sp());
    other.registers_.flags() = registers_.flags();
    other.timeSliceLeft_ = timeSliceLeft_;
    other.halted_ = halted_;
    other.shouldYield_ = shouldYield_;
}

void CpuContext::restoreFrom(const CpuContext& other) {
    registers_.setGpr(other.registers_.gpr());
    registers_.setPc(other.registers_.pc());
    registers_.setSp(other.registers_.sp());
    registers_.flags() = other.registers_.flags();
    timeSliceLeft_ = other.timeSliceLeft_;
    halted_ = other.halted_;
    shouldYield_ = other.shouldYield_;
}

}  // namespace vm
