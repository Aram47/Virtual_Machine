#include "vm/cpu/register_file.hpp"

namespace vm {

RegisterFile::RegisterFile() = default;

Word RegisterFile::get(std::uint8_t index) const {
    if (index >= kRegisterCount) {
        throw VmException("invalid register index");
    }
    return gpr_[index];
}

void RegisterFile::set(std::uint8_t index, Word value) {
    if (index >= kRegisterCount) {
        throw VmException("invalid register index");
    }
    gpr_[index] = value;
}

void RegisterFile::updateFlags(Word result) {
    flags_.zero = (result == 0);
    flags_.sign = (static_cast<std::int32_t>(result) < 0);
}

void RegisterFile::updateFlagsSub(Word lhs, Word rhs, Word result) {
    updateFlags(result);
    flags_.carry = lhs < rhs;
    const bool lhsSign = static_cast<std::int32_t>(lhs) < 0;
    const bool rhsSign = static_cast<std::int32_t>(rhs) < 0;
    const bool resultSign = static_cast<std::int32_t>(result) < 0;
    flags_.overflow = (lhsSign != rhsSign) && (lhsSign != resultSign);
}

}  // namespace vm
