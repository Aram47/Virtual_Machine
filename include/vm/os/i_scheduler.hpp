#pragma once

#include "vm/process/process_control_block.hpp"

#include <vector>

namespace vm {

class VirtualCpuCore;

class IScheduler {
public:
    virtual ~IScheduler() = default;
    virtual void onCoreIdle(std::size_t coreId) = 0;
    virtual void onPreempt(std::size_t coreId, ProcessControlBlock& process) = 0;
    virtual ProcessControlBlock* acquireProcess(std::size_t coreId) = 0;
    virtual void notifyProcessReady() = 0;
    virtual bool hasRunnableWork() const = 0;
};

}  // namespace vm
