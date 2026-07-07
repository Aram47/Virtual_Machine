#pragma once

#include "vm/types.hpp"

#include <string>

namespace vm {

class OpcodeTable {
public:
    static Opcode mnemonicToOpcode(const std::string& name);
    static std::string opcodeToMnemonic(Opcode opcode);
};

}  // namespace vm
