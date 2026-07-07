#pragma once

#include "vm/memory/i_memory.hpp"

#include <mutex>
#include <vector>

namespace vm {

class PhysicalMemory : public IMemory {
public:
    explicit PhysicalMemory(std::size_t size = kDefaultPhysicalMemorySize);

    std::byte readByte(Address address) override;
    void writeByte(Address address, std::byte value) override;
    Word readWord(Address address) override;
    void writeWord(Address address, Word value) override;
    void readBytes(Address address, std::byte* buffer, std::size_t size) override;
    void writeBytes(Address address, const std::byte* buffer, std::size_t size) override;

    Address allocateRegion(std::size_t size);
    std::size_t size() const { return memory_.size(); }

private:
    void validateRange(Address address, std::size_t size) const;

    mutable std::mutex mutex_;
    std::vector<std::byte> memory_;
    Address nextFreeAddress_;
};

}  // namespace vm
