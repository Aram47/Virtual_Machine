#pragma once

#include "vm/types.hpp"

#include <array>

namespace vm {

class RegisterFile {
public:
    RegisterFile();

    Word get(std::uint8_t index) const;
    void set(std::uint8_t index, Word value);

    Address pc() const { return pc_; }
    void setPc(Address pc) { pc_ = pc; }
    void advancePc() { ++pc_; }

    Address sp() const { return sp_; }
    Address& sp() { return sp_; }
    void setSp(Address sp) { sp_ = sp; }

    const CpuFlags& flags() const { return flags_; }
    CpuFlags& flags() { return flags_; }

    void updateFlags(Word result);
    void updateFlagsSub(Word lhs, Word rhs, Word result);

    const std::array<Word, kRegisterCount>& gpr() const { return gpr_; }
    void setGpr(const std::array<Word, kRegisterCount>& gpr) { gpr_ = gpr; }

private:
    std::array<Word, kRegisterCount> gpr_{};
    Address pc_ = 0;
    Address sp_ = 0;
    CpuFlags flags_{};
};

}  // namespace vm
