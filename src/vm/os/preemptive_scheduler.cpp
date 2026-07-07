#include "vm/os/preemptive_scheduler.hpp"

#include <algorithm>

namespace vm {

PreemptiveScheduler::PreemptiveScheduler(std::vector<ProcessControlBlock*>& processes,
                                         std::size_t quantum)
    : processes_(processes), quantum_(quantum) {}

void PreemptiveScheduler::onCoreIdle(std::size_t) {
    notifyProcessReady();
}

void PreemptiveScheduler::onPreempt(std::size_t, ProcessControlBlock& process) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (process.state() == ProcessState::Running) {
        process.setState(ProcessState::Ready);
        readyQueue_.push_back(process.id());
    }
    cv_.notify_all();
}

ProcessControlBlock* PreemptiveScheduler::acquireProcess(std::size_t) {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this]() { return !running_ || !readyQueue_.empty(); });
    if (!running_) {
        return nullptr;
    }
    return pickNextProcess();
}

void PreemptiveScheduler::notifyProcessReady() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto* process : processes_) {
        if (process->state() == ProcessState::Ready) {
            const auto it = std::find(readyQueue_.begin(), readyQueue_.end(), process->id());
            if (it == readyQueue_.end()) {
                readyQueue_.push_back(process->id());
            }
        }
    }
    cv_.notify_all();
}

bool PreemptiveScheduler::hasRunnableWorkLocked() const {
    for (const auto* process : processes_) {
        if (process->state() == ProcessState::Ready || process->state() == ProcessState::Running) {
            return true;
        }
    }
    return false;
}

bool PreemptiveScheduler::hasRunnableWork() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return hasRunnableWorkLocked();
}

void PreemptiveScheduler::stop() {
    running_ = false;
    cv_.notify_all();
}

ProcessControlBlock* PreemptiveScheduler::pickNextProcess() {
    while (!readyQueue_.empty()) {
        const ProcessId id = readyQueue_.front();
        readyQueue_.pop_front();
        for (auto* process : processes_) {
            if (process->id() == id && process->state() == ProcessState::Ready) {
                process->setState(ProcessState::Running);
                process->context().resetTimeSlice(quantum_);
                return process;
            }
        }
    }

    for (std::size_t i = 0; i < processes_.size(); ++i) {
        const std::size_t index = (roundRobinIndex_ + i) % processes_.size();
        auto* process = processes_[index];
        if (process->state() == ProcessState::Ready) {
            roundRobinIndex_ = index + 1;
            process->setState(ProcessState::Running);
            process->context().resetTimeSlice(quantum_);
            return process;
        }
    }
    return nullptr;
}

}  // namespace vm
