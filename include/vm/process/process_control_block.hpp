#pragma once

#include "vm/cpu/cpu_context.hpp"
#include "vm/memory/process_memory.hpp"
#include "vm/process/process_state.hpp"
#include "vm/types.hpp"

#include <memory>
#include <optional>
#include <string>

namespace vm {

class ProcessControlBlock {
public:
    ProcessControlBlock(ProcessId id, std::string name, std::unique_ptr<ProcessMemory> memory);

    ProcessId id() const { return id_; }
    const std::string& name() const { return name_; }

    ProcessState state() const { return state_; }
    void setState(ProcessState state) { state_ = state; }

    CpuContext& context() { return context_; }
    const CpuContext& context() const { return context_; }

    ProcessMemory& memory() { return *memory_; }
    const ProcessMemory& memory() const { return *memory_; }

    void setLastError(std::string message) { lastError_ = std::move(message); }
    void clearLastError() { lastError_.reset(); }
    const std::optional<std::string>& lastError() const { return lastError_; }

private:
    ProcessId id_;
    std::string name_;
    ProcessState state_ = ProcessState::Ready;
    CpuContext context_;
    std::unique_ptr<ProcessMemory> memory_;
    std::optional<std::string> lastError_;
};

}  // namespace vm
