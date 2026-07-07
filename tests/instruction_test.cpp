#include <gtest/gtest.h>

#include "vm/cpu/cpu_context.hpp"
#include "vm/isa/encode.hpp"
#include "vm/isa/instruction_registry.hpp"
#include "vm/memory/physical_memory.hpp"
#include "vm/memory/process_memory.hpp"

using namespace vm;

namespace {

struct CpuFixture {
    PhysicalMemory physical{65536};
    MemoryRegionLayout layout{};
    std::unique_ptr<ProcessMemory> memory;
    CpuContext cpu;
    InstructionRegistry registry;

    CpuFixture() {
        layout.base = physical.allocateRegion(layout.totalSize);
        layout.codeSize = 256;
        layout.dataOffset = 256;
        layout.dataSize = 256;
        layout.stackOffset = 512;
        layout.stackSize = 256;
        memory = std::make_unique<ProcessMemory>(physical, layout);
    }

    ExecutionResult run(const DecodedInstruction& instruction) {
        ExecutionContext ctx{cpu, *memory};
        return registry.execute(instruction, ctx);
    }
};

}  // namespace

TEST(InstructionTest, MovImmediate) {
    CpuFixture fixture;
    const auto instruction = decodeInstruction(
        encodeInstruction(Opcode::MOV, 0, 0, kImmediateMarker, 25));
    EXPECT_EQ(fixture.run(instruction), ExecutionResult::Continue);
    EXPECT_EQ(fixture.cpu.registers().get(0), 25u);
}

TEST(InstructionTest, AddRegisters) {
    CpuFixture fixture;
    fixture.cpu.registers().set(0, 10);
    fixture.cpu.registers().set(1, 5);
    const auto instruction =
        decodeInstruction(encodeInstruction(Opcode::ADD, 0, 0, 1, 0));
    EXPECT_EQ(fixture.run(instruction), ExecutionResult::Continue);
    EXPECT_EQ(fixture.cpu.registers().get(0), 15u);
}

TEST(InstructionTest, LoadAndStore) {
    CpuFixture fixture;
    fixture.memory->writeDataWord(4, 77);

    auto load = decodeInstruction(encodeInstruction(Opcode::LOAD, 2, 0, 0, 4));
    EXPECT_EQ(fixture.run(load), ExecutionResult::Continue);
    EXPECT_EQ(fixture.cpu.registers().get(2), 77u);

    auto store = decodeInstruction(encodeInstruction(Opcode::STORE, 2, 0, 0, 8));
    EXPECT_EQ(fixture.run(store), ExecutionResult::Continue);
    EXPECT_EQ(fixture.memory->readDataWord(8), 77u);
}

TEST(InstructionTest, ConditionalJump) {
    CpuFixture fixture;
    fixture.cpu.registers().updateFlagsSub(1, 2, static_cast<Word>(-1));
    const auto instruction =
        decodeInstruction(encodeInstruction(Opcode::JL, 0, 0, 0, 10));
    EXPECT_EQ(fixture.run(instruction), ExecutionResult::Jump);
    EXPECT_EQ(fixture.cpu.registers().pc(), 10u);
}

TEST(InstructionTest, Halt) {
    CpuFixture fixture;
    const auto instruction = decodeInstruction(encodeInstruction(Opcode::HALT, 0, 0, 0, 0));
    EXPECT_EQ(fixture.run(instruction), ExecutionResult::Halt);
    EXPECT_TRUE(fixture.cpu.halted());
}

TEST(InstructionTest, SubRegisters) {
    CpuFixture fixture;
    fixture.cpu.registers().set(0, 10);
    fixture.cpu.registers().set(1, 3);
    const auto instruction =
        decodeInstruction(encodeInstruction(Opcode::SUB, 0, 0, 1, 0));
    EXPECT_EQ(fixture.run(instruction), ExecutionResult::Continue);
    EXPECT_EQ(fixture.cpu.registers().get(0), 7u);
}

TEST(InstructionTest, MulRegisters) {
    CpuFixture fixture;
    fixture.cpu.registers().set(0, 6);
    fixture.cpu.registers().set(1, 7);
    const auto instruction =
        decodeInstruction(encodeInstruction(Opcode::MUL, 0, 0, 1, 0));
    EXPECT_EQ(fixture.run(instruction), ExecutionResult::Continue);
    EXPECT_EQ(fixture.cpu.registers().get(0), 42u);
}

TEST(InstructionTest, DivByZeroReturnsError) {
    CpuFixture fixture;
    fixture.cpu.registers().set(0, 10);
    fixture.cpu.registers().set(1, 0);
    const auto instruction =
        decodeInstruction(encodeInstruction(Opcode::DIV, 0, 0, 1, 0));
    EXPECT_EQ(fixture.run(instruction), ExecutionResult::Error);
}

TEST(InstructionTest, ConditionalJumps) {
    CpuFixture fixture;
    fixture.cpu.registers().updateFlagsSub(5, 5, 0);

    auto je = decodeInstruction(encodeInstruction(Opcode::JE, 0, 0, 0, 20));
    EXPECT_EQ(fixture.run(je), ExecutionResult::Jump);
    EXPECT_EQ(fixture.cpu.registers().pc(), 20u);

    fixture.cpu.registers().setPc(0);
    fixture.cpu.registers().updateFlagsSub(5, 3, 2);
    auto jne = decodeInstruction(encodeInstruction(Opcode::JNE, 0, 0, 0, 30));
    EXPECT_EQ(fixture.run(jne), ExecutionResult::Jump);
    EXPECT_EQ(fixture.cpu.registers().pc(), 30u);
}

TEST(InstructionTest, PushPopRet) {
    CpuFixture fixture;
    fixture.cpu.registers().setSp(fixture.memory->stackAddress() + fixture.layout.stackSize -
                                  sizeof(Word));

    auto push = decodeInstruction(encodeInstruction(Opcode::PUSH, 0, 0, kImmediateMarker, 99));
    EXPECT_EQ(fixture.run(push), ExecutionResult::Continue);

    auto pop = decodeInstruction(encodeInstruction(Opcode::POP, 1, 0, 0, 0));
    EXPECT_EQ(fixture.run(pop), ExecutionResult::Continue);
    EXPECT_EQ(fixture.cpu.registers().get(1), 99u);

    auto pushRet = decodeInstruction(encodeInstruction(Opcode::PUSH, 0, 0, kImmediateMarker, 7));
    EXPECT_EQ(fixture.run(pushRet), ExecutionResult::Continue);
    auto ret = decodeInstruction(encodeInstruction(Opcode::RET, 0, 0, 0, 0));
    EXPECT_EQ(fixture.run(ret), ExecutionResult::Jump);
    EXPECT_EQ(fixture.cpu.registers().pc(), 7u);
}
