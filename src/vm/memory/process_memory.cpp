#include "vm/memory/process_memory.hpp"

namespace vm {

namespace {

void throwSegmentError(const char* segment, Address address) {
    throw VmException(std::string(segment) + " segment access out of bounds at offset " +
                      std::to_string(address));
}

}  // namespace

ProcessMemory::ProcessMemory(PhysicalMemory& physical, const MemoryRegionLayout& layout,
                             std::vector<std::byte> initialData)
    : physical_(physical), layout_(layout) {
    if (!initialData.empty()) {
        validateDataOffset(0, initialData.size());
        physical_.writeBytes(dataAddress(), initialData.data(), initialData.size());
    }
}

void ProcessMemory::validateDataOffset(Address offset, std::size_t size) const {
    const std::size_t end = static_cast<std::size_t>(offset) + size;
    if (end > layout_.dataSize) {
        throwSegmentError("data", offset);
    }
}

void ProcessMemory::validateStackAccess(Address sp, std::size_t size) const {
    const Address stackBase = stackAddress();
    const Address stackEnd = stackBase + layout_.stackSize;
    if (sp < stackBase || static_cast<std::size_t>(sp - stackBase) + size > layout_.stackSize) {
        throwSegmentError("stack", sp);
    }
}

Word ProcessMemory::readCodeWord(Address pc) const {
    return physical_.readWord(codeAddress() + pc * sizeof(Word));
}

void ProcessMemory::writeCodeWord(Address pc, Word value) {
    physical_.writeWord(codeAddress() + pc * sizeof(Word), value);
}

Word ProcessMemory::readDataWord(Address offset) const {
    validateDataOffset(offset, sizeof(Word));
    return physical_.readWord(dataAddress() + offset);
}

void ProcessMemory::writeDataWord(Address offset, Word value) {
    validateDataOffset(offset, sizeof(Word));
    physical_.writeWord(dataAddress() + offset, value);
}

std::byte ProcessMemory::readDataByte(Address offset) const {
    validateDataOffset(offset, 1);
    return physical_.readByte(dataAddress() + offset);
}

void ProcessMemory::writeDataByte(Address offset, std::byte value) {
    validateDataOffset(offset, 1);
    physical_.writeByte(dataAddress() + offset, value);
}

void ProcessMemory::loadCode(const std::vector<Word>& code) {
    for (std::size_t i = 0; i < code.size(); ++i) {
        writeCodeWord(static_cast<Address>(i), code[i]);
    }
}

void ProcessMemory::loadData(const std::vector<std::byte>& data) {
    validateDataOffset(0, data.size());
    physical_.writeBytes(dataAddress(), data.data(), data.size());
}

bool ProcessMemory::canPush(Address sp) const {
    return sp >= stackAddress() + sizeof(Word) &&
           sp <= stackAddress() + layout_.stackSize;
}

bool ProcessMemory::canPop(Address sp) const {
    return sp + sizeof(Word) <= stackAddress() + layout_.stackSize &&
           sp >= stackAddress();
}

void ProcessMemory::pushWord(Word value, Address& sp) {
    if (!canPush(sp)) {
        throw VmException("stack overflow");
    }
    sp -= sizeof(Word);
    validateStackAccess(sp, sizeof(Word));
    physical_.writeWord(sp, value);
}

Word ProcessMemory::popWord(Address& sp) {
    if (!canPop(sp)) {
        throw VmException("stack underflow");
    }
    validateStackAccess(sp, sizeof(Word));
    const Word value = physical_.readWord(sp);
    sp += sizeof(Word);
    return value;
}

}  // namespace vm
