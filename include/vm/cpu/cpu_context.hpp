#pragma once

#include "vm/cpu/register_file.hpp"

namespace vm {

class CpuContext {
public:
    CpuContext();

    RegisterFile& registers() { return registers_; }
    const RegisterFile& registers() const { return registers_; }

    std::size_t timeSliceLeft() const { return timeSliceLeft_; }
    void setTimeSliceLeft(std::size_t value) { timeSliceLeft_ = value; }
    void resetTimeSlice(std::size_t quantum) { timeSliceLeft_ = quantum; }

    bool halted() const { return halted_; }
    void setHalted(bool value) { halted_ = value; }

    bool shouldYield() const { return shouldYield_; }
    void setShouldYield(bool value) { shouldYield_ = value; }

    void saveTo(CpuContext& other) const;
    void restoreFrom(const CpuContext& other);

private:
    RegisterFile registers_;
    std::size_t timeSliceLeft_ = kDefaultTimeSlice;
    bool halted_ = false;
    bool shouldYield_ = false;
};

}  // namespace vm
