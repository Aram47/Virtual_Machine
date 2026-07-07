#pragma once

#include "vm/process/process_control_block.hpp"
#include "vm/types.hpp"

#include <memory>
#include <string>
#include <vector>

namespace vm {

class ProcessTable {
public:
    ProcessControlBlock& emplace(std::unique_ptr<ProcessControlBlock> process);
    ProcessControlBlock* find(ProcessId id);
    const ProcessControlBlock* find(ProcessId id) const;

    bool allHalted() const;
    bool empty() const { return processes_.empty(); }

    std::vector<ProcessControlBlock*>& refs() { return refs_; }
    const std::vector<ProcessControlBlock*>& refs() const { return refs_; }

    ProcessId nextId() { return nextProcessId_++; }

private:
    std::vector<std::unique_ptr<ProcessControlBlock>> processes_;
    std::vector<ProcessControlBlock*> refs_;
    ProcessId nextProcessId_ = 1;
};

}  // namespace vm
