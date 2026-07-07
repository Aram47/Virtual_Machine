#pragma once

#include "vm/os/i_scheduler.hpp"

#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <vector>

namespace vm {

class PreemptiveScheduler : public IScheduler {
public:
  PreemptiveScheduler(std::vector<ProcessControlBlock*>& processes, std::size_t quantum);

    void onCoreIdle(std::size_t coreId) override;
    void onPreempt(std::size_t coreId, ProcessControlBlock& process) override;
    ProcessControlBlock* acquireProcess(std::size_t coreId) override;
    void notifyProcessReady() override;
    bool hasRunnableWork() const override;

    void stop();

private:
    ProcessControlBlock* pickNextProcess();
    bool hasRunnableWorkLocked() const;

    std::vector<ProcessControlBlock*>& processes_;
    std::size_t quantum_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::deque<ProcessId> readyQueue_;
    std::size_t roundRobinIndex_ = 0;
    std::atomic<bool> running_{true};
};

}  // namespace vm
