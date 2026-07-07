#pragma once

#include "vm/assembler/parser.hpp"
#include "vm/memory/physical_memory.hpp"
#include "vm/memory/process_memory.hpp"
#include "vm/types.hpp"

#include <memory>
#include <string>

namespace vm {

struct LoadedProcessResources {
    std::unique_ptr<ProcessMemory> memory;
    MemoryRegionLayout layout;
};

class ProgramLoader {
public:
    explicit ProgramLoader(PhysicalMemory& physicalMemory);

    LoadedProcessResources load(const AssembledProgram& program);

private:
    PhysicalMemory& physicalMemory_;
};

}  // namespace vm
