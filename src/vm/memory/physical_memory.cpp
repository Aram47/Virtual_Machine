#include "vm/memory/physical_memory.hpp"

namespace vm {

PhysicalMemory::PhysicalMemory(std::size_t size)
    : memory_(size, std::byte{0}), nextFreeAddress_(0) {}

void PhysicalMemory::validateRange(Address address, std::size_t size) const {
    const std::size_t end = static_cast<std::size_t>(address) + size;
    if (end > memory_.size()) {
        throw VmException("memory access out of bounds at address " + std::to_string(address));
    }
}

std::byte PhysicalMemory::readByte(Address address) {
    std::lock_guard<std::mutex> lock(mutex_);
    validateRange(address, 1);
    return memory_[address];
}

void PhysicalMemory::writeByte(Address address, std::byte value) {
    std::lock_guard<std::mutex> lock(mutex_);
    validateRange(address, 1);
    memory_[address] = value;
}

Word PhysicalMemory::readWord(Address address) {
    std::lock_guard<std::mutex> lock(mutex_);
    validateRange(address, sizeof(Word));
    Word value = 0;
    for (std::size_t i = 0; i < sizeof(Word); ++i) {
        value |= static_cast<Word>(static_cast<std::uint8_t>(memory_[address + i])) << (8 * i);
    }
    return value;
}

void PhysicalMemory::writeWord(Address address, Word value) {
    std::lock_guard<std::mutex> lock(mutex_);
    validateRange(address, sizeof(Word));
    for (std::size_t i = 0; i < sizeof(Word); ++i) {
        memory_[address + i] = static_cast<std::byte>((value >> (8 * i)) & 0xFF);
    }
}

void PhysicalMemory::readBytes(Address address, std::byte* buffer, std::size_t size) {
    std::lock_guard<std::mutex> lock(mutex_);
    validateRange(address, size);
    for (std::size_t i = 0; i < size; ++i) {
        buffer[i] = memory_[address + i];
    }
}

void PhysicalMemory::writeBytes(Address address, const std::byte* buffer, std::size_t size) {
    std::lock_guard<std::mutex> lock(mutex_);
    validateRange(address, size);
    for (std::size_t i = 0; i < size; ++i) {
        memory_[address + i] = buffer[i];
    }
}

Address PhysicalMemory::allocateRegion(std::size_t size) {
    std::lock_guard<std::mutex> lock(mutex_);
    const Address base = nextFreeAddress_;
    if (static_cast<std::size_t>(base) + size > memory_.size()) {
        throw VmException("not enough physical memory to allocate region");
    }
    nextFreeAddress_ += static_cast<Address>(size);
    return base;
}

}  // namespace vm
