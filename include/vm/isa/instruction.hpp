#pragma once

#include "vm/types.hpp"

namespace vm {

class CpuContext;
class ProcessControlBlock;
class ProcessMemory;

enum class ExecutionResult {
    Continue,
    Halt,
    Yield,
    Jump,
    Error,
};

struct ExecutionContext {
    CpuContext& cpu;
    ProcessMemory& memory;
    ProcessControlBlock* process = nullptr;
};

}  // namespace vm
