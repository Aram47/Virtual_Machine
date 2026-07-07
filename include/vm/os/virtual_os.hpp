#pragma once

#include "vm/debug/i_execution_tracer.hpp"
#include "vm/assembler/assembler.hpp"
#include "vm/cpu/virtual_cpu_core.hpp"
#include "vm/isa/instruction_registry.hpp"
#include "vm/memory/physical_memory.hpp"
#include "vm/os/preemptive_scheduler.hpp"
#include "vm/os/program_loader.hpp"
#include "vm/os/syscall_handler.hpp"
#include "vm/process/process_control_block.hpp"
#include "vm/process/process_table.hpp"
#include "vm/types.hpp"

#include <condition_variable>
#include <fstream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

namespace vm {

class VirtualOS {
public:
    VirtualOS(std::size_t coreCount, std::size_t quantum);

    ProcessId loadProgram(const std::string& path);
    ProcessId loadProgramFromSource(const std::string& source, const std::string& name);

    void run();
    void stop();
    bool isRunning() const { return running_; }

    std::size_t coreCount() const { return cores_.size(); }
    std::size_t quantum() const { return quantum_; }

    ProcessControlBlock* findProcess(ProcessId id);
    const ProcessControlBlock* findProcess(ProcessId id) const;

    ExecutionResult stepProcess(ProcessId id);
    Word getRegister(ProcessId id, std::uint8_t reg) const;

    void setTracer(IExecutionTracer* tracer);

    std::vector<ProcessControlBlock*> processPointers();

private:
    ProcessControlBlock& createProcess(const std::string& name, const AssembledProgram& program);
    void notifyPossibleShutdown();

    PhysicalMemory physicalMemory_;
    ProgramLoader programLoader_;
    ProcessTable processTable_;
    InstructionRegistry instructionRegistry_;
    SyscallHandler syscallHandler_;
    Assembler assembler_;

    std::unique_ptr<PreemptiveScheduler> scheduler_;
    std::vector<std::unique_ptr<VirtualCpuCore>> cores_;

    std::size_t quantum_;
    bool running_ = false;
    IExecutionTracer* tracer_ = nullptr;

    mutable std::mutex shutdownMutex_;
    std::condition_variable shutdownCv_;
};

}  // namespace vm
