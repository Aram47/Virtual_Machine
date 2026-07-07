#include <gtest/gtest.h>

#include "vm/os/syscall_handler.hpp"
#include "vm/process/process_control_block.hpp"
#include "vm/memory/physical_memory.hpp"
#include "vm/memory/process_memory.hpp"

#include <sstream>

using namespace vm;

TEST(SyscallTest, PrintWritesToStream) {
    PhysicalMemory physical(65536);
    MemoryRegionLayout layout{};
    layout.base = physical.allocateRegion(layout.totalSize);
    layout.dataOffset = 256;
    layout.dataSize = 256;
    layout.stackOffset = 512;
    auto memory = std::make_unique<ProcessMemory>(physical, layout);
    memory->writeDataWord(0, 123);

    ProcessControlBlock process(1, "test", std::move(memory));
    std::ostringstream out;
    SyscallHandler handler(out);
    handler.handle(SyscallId::Print, process, 0);
    EXPECT_EQ(out.str(), "123\n");
}

TEST(SyscallTest, ExitHaltsProcess) {
    PhysicalMemory physical(65536);
    MemoryRegionLayout layout{};
    layout.base = physical.allocateRegion(layout.totalSize);
    layout.dataSize = 256;
    auto memory = std::make_unique<ProcessMemory>(physical, layout);
    ProcessControlBlock process(1, "test", std::move(memory));

    SyscallHandler handler;
    handler.handle(SyscallId::Exit, process, 0);
    EXPECT_EQ(process.state(), ProcessState::Halted);
    EXPECT_TRUE(process.context().halted());
}

TEST(SyscallTest, YieldSetsFlag) {
    PhysicalMemory physical(65536);
    MemoryRegionLayout layout{};
    layout.base = physical.allocateRegion(layout.totalSize);
    layout.dataSize = 256;
    auto memory = std::make_unique<ProcessMemory>(physical, layout);
    ProcessControlBlock process(1, "test", std::move(memory));

    SyscallHandler handler;
    handler.handle(SyscallId::Yield, process, 0);
    EXPECT_TRUE(process.context().shouldYield());
}

TEST(SyscallTest, ScanReadsFromStream) {
    PhysicalMemory physical(65536);
    MemoryRegionLayout layout{};
    layout.base = physical.allocateRegion(layout.totalSize);
    layout.dataSize = 256;
    auto memory = std::make_unique<ProcessMemory>(physical, layout);
    ProcessControlBlock process(1, "test", std::move(memory));

    std::istringstream in("456");
    std::ostringstream out;
    SyscallHandler handler(out, in);
    handler.handle(SyscallId::Scan, process, 0);
    EXPECT_EQ(process.memory().readDataWord(0), 456u);
}

TEST(SyscallTest, ScanFailsOnBadInput) {
    PhysicalMemory physical(65536);
    MemoryRegionLayout layout{};
    layout.base = physical.allocateRegion(layout.totalSize);
    layout.dataSize = 256;
    auto memory = std::make_unique<ProcessMemory>(physical, layout);
    ProcessControlBlock process(1, "test", std::move(memory));

    std::istringstream in("abc");
    SyscallHandler handler(std::cout, in);
    EXPECT_THROW(handler.handle(SyscallId::Scan, process, 0), VmException);
}

TEST(SyscallTest, LoadInvokesCallback) {
    PhysicalMemory physical(65536);
    MemoryRegionLayout layout{};
    layout.base = physical.allocateRegion(layout.totalSize);
    layout.dataSize = 256;
    auto memory = std::make_unique<ProcessMemory>(physical, layout);
    const std::string path = "program.asm";
    for (std::size_t i = 0; i < path.size(); ++i) {
        memory->writeDataByte(static_cast<Address>(i), static_cast<std::byte>(path[i]));
    }
    memory->writeDataByte(static_cast<Address>(path.size()), std::byte{0});

    ProcessControlBlock process(1, "test", std::move(memory));
    SyscallHandler handler;
    std::string loadedPath;
    handler.setLoadCallback([&loadedPath](const std::string& p) {
        loadedPath = p;
        return 2;
    });
    handler.handle(SyscallId::Load, process, 0);
    EXPECT_EQ(loadedPath, path);
}

TEST(SyscallTest, LoadWithoutCallbackThrows) {
    PhysicalMemory physical(65536);
    MemoryRegionLayout layout{};
    layout.base = physical.allocateRegion(layout.totalSize);
    layout.dataSize = 256;
    auto memory = std::make_unique<ProcessMemory>(physical, layout);
    ProcessControlBlock process(1, "test", std::move(memory));

    SyscallHandler handler;
    EXPECT_THROW(handler.handle(SyscallId::Load, process, 0), VmException);
}
