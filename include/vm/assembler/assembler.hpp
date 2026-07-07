#pragma once

#include "vm/assembler/parser.hpp"

namespace vm {

class Assembler {
public:
    AssembledProgram assemble(const std::string& source) { return parser_.parse(source); }

private:
    Parser parser_;
};

}  // namespace vm
