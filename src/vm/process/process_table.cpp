#include "vm/process/process_table.hpp"

namespace vm {

ProcessControlBlock& ProcessTable::emplace(std::unique_ptr<ProcessControlBlock> process) {
    ProcessControlBlock& ref = *process;
    processes_.push_back(std::move(process));
    refs_.push_back(&ref);
    return ref;
}

ProcessControlBlock* ProcessTable::find(ProcessId id) {
    for (auto& process : processes_) {
        if (process->id() == id) {
            return process.get();
        }
    }
    return nullptr;
}

const ProcessControlBlock* ProcessTable::find(ProcessId id) const {
    for (const auto& process : processes_) {
        if (process->id() == id) {
            return process.get();
        }
    }
    return nullptr;
}

bool ProcessTable::allHalted() const {
    if (processes_.empty()) {
        return true;
    }
    for (const auto& process : processes_) {
        if (process->state() != ProcessState::Halted) {
            return false;
        }
    }
    return true;
}

}  // namespace vm
