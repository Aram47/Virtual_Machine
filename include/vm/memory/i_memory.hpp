#pragma once

#include "vm/types.hpp"

#include <cstddef>
#include <vector>

namespace vm {

class IMemory {
public:
    virtual ~IMemory() = default;

    virtual std::byte readByte(Address address) = 0;
    virtual void writeByte(Address address, std::byte value) = 0;
    virtual Word readWord(Address address) = 0;
    virtual void writeWord(Address address, Word value) = 0;

    virtual void readBytes(Address address, std::byte* buffer, std::size_t size) = 0;
    virtual void writeBytes(Address address, const std::byte* buffer, std::size_t size) = 0;
};

struct MemoryRegionLayout {
    Address base = 0;
    Address codeOffset = 0;
    Address dataOffset = 0;
    Address stackOffset = 0;
    std::size_t codeSize = 0;
    std::size_t dataSize = 0;
    std::size_t stackSize = kStackSize;
    std::size_t totalSize = kDefaultProcessMemorySize;
};

}  // namespace vm
