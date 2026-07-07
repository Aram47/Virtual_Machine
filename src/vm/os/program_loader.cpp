#include "vm/os/program_loader.hpp"

namespace vm {

ProgramLoader::ProgramLoader(PhysicalMemory& physicalMemory) : physicalMemory_(physicalMemory) {}

LoadedProcessResources ProgramLoader::load(const AssembledProgram& program) {
    MemoryRegionLayout layout{};
    layout.base = physicalMemory_.allocateRegion(layout.totalSize);
    layout.codeSize = program.code.size() * sizeof(Word);
    layout.dataSize = program.data.size();
    layout.dataOffset = layout.codeSize;
    layout.stackOffset = layout.codeSize + layout.dataSize;

    auto processMemory =
        std::make_unique<ProcessMemory>(physicalMemory_, layout, program.data);
    processMemory->loadCode(program.code);

    LoadedProcessResources resources;
    resources.memory = std::move(processMemory);
    resources.layout = layout;
    return resources;
}

}  // namespace vm
