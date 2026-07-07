#pragma once

#include "vm/cpu/cpu_context.hpp"
#include "vm/debug/i_execution_tracer.hpp"
#include "vm/isa/instruction_registry.hpp"
#include "vm/os/i_scheduler.hpp"
#include "vm/os/syscall_handler.hpp"
#include "vm/process/process_control_block.hpp"

#include <atomic>
#include <functional>
#include <memory>
#include <thread>

namespace vm {

class VirtualCpuCore {
public:
    VirtualCpuCore(std::size_t coreId, InstructionRegistry& registry, ISyscallHandler& syscalls,
                   IScheduler& scheduler, std::size_t quantum);

    void start();
    void stop();
    void join();

    void setOnProcessHalted(std::function<void()> callback) { onProcessHalted_ = std::move(callback); }
    void setTracer(IExecutionTracer* tracer) { tracer_ = tracer; }

    bool isRunning() const { return running_.load(); }

    ProcessControlBlock* currentProcess() const { return currentProcess_; }

    ExecutionResult runSingleStep(ProcessControlBlock& process);

private:
    void coreLoop();
    ExecutionResult executeInstruction(ProcessControlBlock& process);
    void releaseProcess(ProcessControlBlock& process, bool preempt);

    std::size_t coreId_;
    InstructionRegistry& registry_;
    ISyscallHandler& syscalls_;
    IScheduler& scheduler_;
    std::size_t quantum_;

    std::thread thread_;
    std::atomic<bool> running_{false};
    std::atomic<bool> stopRequested_{false};

    ProcessControlBlock* currentProcess_ = nullptr;
    std::function<void()> onProcessHalted_;
    IExecutionTracer* tracer_ = nullptr;
};

}  // namespace vm
