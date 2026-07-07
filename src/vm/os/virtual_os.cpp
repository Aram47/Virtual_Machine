#include "vm/os/virtual_os.hpp"

#include <filesystem>
#include <fstream>

namespace vm {

VirtualOS::VirtualOS(std::size_t coreCount, std::size_t quantum)
    : quantum_(quantum), programLoader_(physicalMemory_) {
    syscallHandler_.setLoadCallback([this](const std::string& path) {
        return loadProgram(path);
    });

    scheduler_ = std::make_unique<PreemptiveScheduler>(processTable_.refs(), quantum_);

    for (std::size_t i = 0; i < coreCount; ++i) {
        auto core = std::make_unique<VirtualCpuCore>(
            i, instructionRegistry_, syscallHandler_, *scheduler_, quantum_);
        core->setOnProcessHalted([this]() { notifyPossibleShutdown(); });
        core->setTracer(tracer_);
        cores_.push_back(std::move(core));
    }
}

ProcessId VirtualOS::loadProgram(const std::string& path) {
    std::ifstream input(path);
    if (!input) {
        throw VmException("failed to open program file: " + path);
    }
    std::stringstream buffer;
    buffer << input.rdbuf();
    const std::string name = std::filesystem::path(path).filename().string();
    return loadProgramFromSource(buffer.str(), name);
}

ProcessId VirtualOS::loadProgramFromSource(const std::string& source, const std::string& name) {
    const AssembledProgram program = assembler_.assemble(source);
    ProcessControlBlock& process = createProcess(name, program);
    process.setState(ProcessState::Ready);
    scheduler_->notifyProcessReady();
    return process.id();
}

ProcessControlBlock& VirtualOS::createProcess(const std::string& name,
                                              const AssembledProgram& program) {
    LoadedProcessResources resources = programLoader_.load(program);

    auto pcb = std::make_unique<ProcessControlBlock>(
        processTable_.nextId(), name, std::move(resources.memory));
    pcb->context().registers().setSp(
        pcb->memory().stackAddress() + kStackSize - sizeof(Word));
    pcb->context().resetTimeSlice(quantum_);

    return processTable_.emplace(std::move(pcb));
}

void VirtualOS::run() {
    if (running_) {
        return;
    }
    running_ = true;
    scheduler_->notifyProcessReady();

    for (auto& core : cores_) {
        core->start();
    }

    {
        std::unique_lock<std::mutex> lock(shutdownMutex_);
        shutdownCv_.wait(lock, [this]() { return processTable_.allHalted(); });
    }

    stop();
}

void VirtualOS::notifyPossibleShutdown() {
    std::lock_guard<std::mutex> lock(shutdownMutex_);
    if (processTable_.allHalted()) {
        shutdownCv_.notify_all();
    }
}

void VirtualOS::stop() {
    if (!running_) {
        return;
    }
    running_ = false;
    scheduler_->stop();
    for (auto& core : cores_) {
        core->stop();
    }
    for (auto& core : cores_) {
        core->join();
    }
    shutdownCv_.notify_all();
}

ProcessControlBlock* VirtualOS::findProcess(ProcessId id) {
    return processTable_.find(id);
}

const ProcessControlBlock* VirtualOS::findProcess(ProcessId id) const {
    return processTable_.find(id);
}

ExecutionResult VirtualOS::stepProcess(ProcessId id) {
    ProcessControlBlock* process = findProcess(id);
    if (!process) {
        throw VmException("process not found");
    }
    process->setState(ProcessState::Running);
    VirtualCpuCore core(0, instructionRegistry_, syscallHandler_, *scheduler_, quantum_);
    core.setTracer(tracer_);
    return core.runSingleStep(*process);
}

Word VirtualOS::getRegister(ProcessId id, std::uint8_t reg) const {
    const ProcessControlBlock* process = findProcess(id);
    if (!process) {
        throw VmException("process not found");
    }
    return process->context().registers().get(reg);
}

std::vector<ProcessControlBlock*> VirtualOS::processPointers() {
    return processTable_.refs();
}

void VirtualOS::setTracer(IExecutionTracer* tracer) {
    tracer_ = tracer;
    for (auto& core : cores_) {
        core->setTracer(tracer);
    }
}

}  // namespace vm
