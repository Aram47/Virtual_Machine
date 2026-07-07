#include <gtest/gtest.h>

#include "vm/os/preemptive_scheduler.hpp"
#include "vm/process/process_control_block.hpp"
#include "vm/memory/physical_memory.hpp"
#include "vm/memory/process_memory.hpp"

using namespace vm;

namespace {

ProcessControlBlock* makeProcess(PhysicalMemory& physical, ProcessId id, const std::string& name) {
    MemoryRegionLayout layout{};
    layout.base = physical.allocateRegion(layout.totalSize);
    layout.codeSize = 64;
    layout.dataOffset = 64;
    layout.dataSize = 64;
    layout.stackOffset = 128;
    auto memory = std::make_unique<ProcessMemory>(physical, layout);
    auto* pcb = new ProcessControlBlock(id, name, std::move(memory));
    pcb->setState(ProcessState::Ready);
    return pcb;
}

}  // namespace

TEST(SchedulerTest, RoundRobinSelection) {
    PhysicalMemory physical(1024 * 1024);
    auto* p1 = makeProcess(physical, 1, "p1");
    auto* p2 = makeProcess(physical, 2, "p2");
    std::vector<ProcessControlBlock*> processes{p1, p2};

    PreemptiveScheduler scheduler(processes, 10);
    scheduler.notifyProcessReady();

    ProcessControlBlock* first = scheduler.acquireProcess(0);
    ASSERT_NE(first, nullptr);
    EXPECT_EQ(first->id(), 1u);

    scheduler.onPreempt(0, *first);
    ProcessControlBlock* second = scheduler.acquireProcess(0);
    ASSERT_NE(second, nullptr);
    EXPECT_EQ(second->id(), 2u);

    delete p1;
    delete p2;
}

TEST(SchedulerTest, HasRunnableWork) {
    PhysicalMemory physical(1024 * 1024);
    auto* process = makeProcess(physical, 1, "p1");
    std::vector<ProcessControlBlock*> processes{process};
    PreemptiveScheduler scheduler(processes, 10);

    EXPECT_TRUE(scheduler.hasRunnableWork());
    process->setState(ProcessState::Halted);
    EXPECT_FALSE(scheduler.hasRunnableWork());

    delete process;
}

TEST(SchedulerTest, PreemptByQuantum) {
    PhysicalMemory physical(1024 * 1024);
    auto* process = makeProcess(physical, 1, "p1");
    std::vector<ProcessControlBlock*> processes{process};
    PreemptiveScheduler scheduler(processes, 2);
    scheduler.notifyProcessReady();

    ProcessControlBlock* acquired = scheduler.acquireProcess(0);
    ASSERT_NE(acquired, nullptr);
    acquired->context().setTimeSliceLeft(0);
    scheduler.onPreempt(0, *acquired);
    EXPECT_EQ(acquired->state(), ProcessState::Ready);

    delete process;
}
