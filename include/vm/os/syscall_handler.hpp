#pragma once

#include "vm/types.hpp"

#include <functional>
#include <iostream>
#include <istream>
#include <mutex>
#include <ostream>
#include <string>

namespace vm {

class ProcessControlBlock;

class ISyscallHandler {
public:
    virtual ~ISyscallHandler() = default;
    virtual void handle(SyscallId id, ProcessControlBlock& process, Word argAddress) = 0;
};

class SyscallHandler : public ISyscallHandler {
public:
    using LoadCallback = std::function<ProcessId(const std::string& path)>;

    SyscallHandler(std::ostream& out = std::cout, std::istream& in = std::cin);

    void setLoadCallback(LoadCallback callback) { loadCallback_ = std::move(callback); }

    void handle(SyscallId id, ProcessControlBlock& process, Word argAddress) override;

private:
    std::ostream& out_;
    std::istream& in_;
    LoadCallback loadCallback_;
    std::mutex outMutex_;
};

}  // namespace vm
