#pragma once

#include "vm/memory/i_memory.hpp"
#include "vm/memory/physical_memory.hpp"

#include <vector>

namespace vm {

class ProcessMemory {
public:
    ProcessMemory(PhysicalMemory& physical, const MemoryRegionLayout& layout,
                  std::vector<std::byte> initialData = {});

    const MemoryRegionLayout& layout() const { return layout_; }

    Address codeAddress() const { return layout_.base + layout_.codeOffset; }
    Address dataAddress() const { return layout_.base + layout_.dataOffset; }
    Address stackAddress() const { return layout_.base + layout_.stackOffset; }

    Word readCodeWord(Address pc) const;
    void writeCodeWord(Address pc, Word value);

    Word readDataWord(Address offset) const;
    void writeDataWord(Address offset, Word value);

    std::byte readDataByte(Address offset) const;
    void writeDataByte(Address offset, std::byte value);

    void loadCode(const std::vector<Word>& code);
    void loadData(const std::vector<std::byte>& data);

    void pushWord(Word value, Address& sp);
    Word popWord(Address& sp);
    bool canPush(Address sp) const;
    bool canPop(Address sp) const;

private:
    void validateDataOffset(Address offset, std::size_t size) const;
    void validateStackAccess(Address sp, std::size_t size) const;

    PhysicalMemory& physical_;
    MemoryRegionLayout layout_;
};

}  // namespace vm
