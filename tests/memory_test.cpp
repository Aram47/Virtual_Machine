#include <gtest/gtest.h>

#include "vm/memory/physical_memory.hpp"
#include "vm/memory/process_memory.hpp"

#include <thread>
#include <vector>

using namespace vm;

TEST(PhysicalMemoryTest, ReadWriteWord) {
    PhysicalMemory memory(1024);
    memory.writeWord(0, 0xAABBCCDD);
    EXPECT_EQ(memory.readWord(0), 0xAABBCCDD);
}

TEST(PhysicalMemoryTest, OutOfBoundsThrows) {
    PhysicalMemory memory(16);
    EXPECT_THROW(memory.readWord(20), VmException);
}

TEST(PhysicalMemoryTest, ConcurrentAccess) {
    constexpr int kIterations = 100;
    constexpr Address kRegionStride = kIterations * sizeof(Word);

    PhysicalMemory memory(4096);
    std::vector<std::thread> threads;
    for (int t = 0; t < 4; ++t) {
        threads.emplace_back([&memory, t, kIterations, kRegionStride]() {
            for (int i = 0; i < kIterations; ++i) {
                const Address address =
                    static_cast<Address>(t * kRegionStride + i * sizeof(Word));
                memory.writeWord(address, static_cast<Word>(t * 1000 + i));
            }
        });
    }
    for (auto& thread : threads) {
        thread.join();
    }
    EXPECT_EQ(memory.readWord(0), 0u);
    EXPECT_EQ(memory.readWord(400), 1000u);
    EXPECT_EQ(memory.readWord(800), 2000u);
    EXPECT_EQ(memory.readWord(1200), 3000u);
}

TEST(ProcessMemoryTest, CodeAndDataSegments) {
    PhysicalMemory physical(65536);
    MemoryRegionLayout layout{};
    layout.base = physical.allocateRegion(layout.totalSize);
    layout.codeSize = 64;
    layout.dataOffset = 64;
    layout.dataSize = 64;
    layout.stackOffset = 128;

    ProcessMemory processMemory(physical, layout);
    processMemory.loadCode({encodeInstruction(Opcode::HALT, 0, 0, 0, 0)});
    processMemory.writeDataWord(0, 99);

    EXPECT_EQ(processMemory.readCodeWord(0), encodeInstruction(Opcode::HALT, 0, 0, 0, 0));
    EXPECT_EQ(processMemory.readDataWord(0), 99u);
}

TEST(ProcessMemoryTest, DataOutOfBoundsThrows) {
    PhysicalMemory physical(65536);
    MemoryRegionLayout layout{};
    layout.base = physical.allocateRegion(layout.totalSize);
    layout.codeSize = 64;
    layout.dataOffset = 64;
    layout.dataSize = 8;
    layout.stackOffset = 72;
    layout.stackSize = 56;

    ProcessMemory processMemory(physical, layout);
    EXPECT_THROW(processMemory.readDataWord(8), VmException);
}

TEST(ProcessMemoryTest, StackPushPop) {
    PhysicalMemory physical(65536);
    MemoryRegionLayout layout{};
    layout.base = physical.allocateRegion(layout.totalSize);
    layout.codeSize = 64;
    layout.dataOffset = 64;
    layout.dataSize = 64;
    layout.stackOffset = 128;
    layout.stackSize = 256;

    ProcessMemory processMemory(physical, layout);
    Address sp = processMemory.stackAddress() + layout.stackSize - sizeof(Word);
    processMemory.pushWord(42, sp);
    EXPECT_EQ(processMemory.popWord(sp), 42u);
}

TEST(ProcessMemoryTest, StackUnderflowThrows) {
    PhysicalMemory physical(65536);
    MemoryRegionLayout layout{};
    layout.base = physical.allocateRegion(layout.totalSize);
    layout.codeSize = 64;
    layout.dataOffset = 64;
    layout.dataSize = 64;
    layout.stackOffset = 128;
    layout.stackSize = 256;

    ProcessMemory processMemory(physical, layout);
    Address sp = processMemory.stackAddress() + layout.stackSize;
    EXPECT_THROW(processMemory.popWord(sp), VmException);
}
