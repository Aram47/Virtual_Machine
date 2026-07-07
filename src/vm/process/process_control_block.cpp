#include "vm/process/process_control_block.hpp"

namespace vm {

ProcessControlBlock::ProcessControlBlock(ProcessId id, std::string name,
                                       std::unique_ptr<ProcessMemory> memory)
    : id_(id), name_(std::move(name)), memory_(std::move(memory)) {}

}  // namespace vm
