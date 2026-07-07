#include <gtest/gtest.h>

#include "vm/assembler/assembler.hpp"
#include "vm/isa/encode.hpp"

using namespace vm;

TEST(AssemblerTest, AssemblesMovAndHalt) {
    const std::string source = R"(
CODE:
    mov r0, 7
    halt
)";
    Assembler assembler;
    const AssembledProgram program = assembler.assemble(source);
    ASSERT_EQ(program.code.size(), 2u);
    EXPECT_EQ(program.code[0],
              encodeInstruction(Opcode::MOV, 0, 0, kImmediateMarker, 7));
    EXPECT_EQ(program.code[1], encodeInstruction(Opcode::HALT, 0, 0, 0, 0));
}

TEST(AssemblerTest, DataSectionAllocatesSymbols) {
    const std::string source = R"(
DATA:
    BYTE flag 1
    WORD value 42
CODE:
    mov r0, 7
    halt
)";
    Assembler assembler;
    const AssembledProgram program = assembler.assemble(source);
    ASSERT_GE(program.data.size(), 5u);
    EXPECT_EQ(static_cast<int>(program.data[0]), 1);
    Word value = 0;
    for (std::size_t i = 0; i < 4; ++i) {
        value |= static_cast<Word>(static_cast<std::uint8_t>(program.data[1 + i])) << (8 * i);
    }
    EXPECT_EQ(value, 42u);
}

TEST(AssemblerTest, LabelResolution) {
    const std::string source = R"(
CODE:
start:
    inc r0
    jmp start
    halt
)";
    Assembler assembler;
    const AssembledProgram program = assembler.assemble(source);
    ASSERT_EQ(program.code.size(), 3u);
    const auto jump = decodeInstruction(program.code[1]);
    EXPECT_EQ(jump.opcode, Opcode::JMP);
    EXPECT_EQ(jump.imm, 0);
}

TEST(AssemblerTest, UnknownOpcodeThrows) {
    const std::string source = R"(
CODE:
    foobar r0
    halt
)";
    Assembler assembler;
    EXPECT_THROW(assembler.assemble(source), AssemblerError);
}

TEST(AssemblerTest, UnknownLabelThrows) {
    const std::string source = R"(
CODE:
    jmp missing
    halt
)";
    Assembler assembler;
    EXPECT_THROW(assembler.assemble(source), AssemblerError);
}

TEST(AssemblerTest, AssemblesPushPopRet) {
    const std::string source = R"(
CODE:
    push 10
    pop r0
    ret
    halt
)";
    Assembler assembler;
    const AssembledProgram program = assembler.assemble(source);
    ASSERT_EQ(program.code.size(), 4u);
    EXPECT_EQ(decodeInstruction(program.code[0]).opcode, Opcode::PUSH);
    EXPECT_EQ(decodeInstruction(program.code[1]).opcode, Opcode::POP);
    EXPECT_EQ(decodeInstruction(program.code[2]).opcode, Opcode::RET);
}
