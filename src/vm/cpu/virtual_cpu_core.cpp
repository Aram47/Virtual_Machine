#include "vm/cpu/virtual_cpu_core.hpp"

#include "vm/isa/encode.hpp"

#include <iostream>

namespace vm {

VirtualCpuCore::VirtualCpuCore(std::size_t coreId, InstructionRegistry& registry,
                               ISyscallHandler& syscalls, IScheduler& scheduler,
                               std::size_t quantum)
    : coreId_(coreId),
      registry_(registry),
      syscalls_(syscalls),
      scheduler_(scheduler),
      quantum_(quantum) {}

void VirtualCpuCore::start() {
    if (running_.exchange(true)) {
        return;
    }
    stopRequested_ = false;
    thread_ = std::thread(&VirtualCpuCore::coreLoop, this);
}

void VirtualCpuCore::stop() {
    stopRequested_ = true;
}

void VirtualCpuCore::join() {
    if (thread_.joinable()) {
        thread_.join();
    }
    running_ = false;
}

ExecutionResult VirtualCpuCore::runSingleStep(ProcessControlBlock& process) {
    currentProcess_ = &process;
    const ExecutionResult result = executeInstruction(process);
    currentProcess_ = nullptr;
    return result;
}

void VirtualCpuCore::coreLoop() {
    ProcessControlBlock* process = nullptr;
    while (!stopRequested_) {
        if (!process) {
            process = scheduler_.acquireProcess(coreId_);
            if (!process) {
                if (stopRequested_) {
                    break;
                }
                scheduler_.onCoreIdle(coreId_);
                continue;
            }
        }

        if (process->context().halted() || process->state() == ProcessState::Halted) {
            releaseProcess(*process, false);
            process = nullptr;
            continue;
        }

        const ExecutionResult result = executeInstruction(*process);

        if (result == ExecutionResult::Halt || process->context().halted()) {
            process->setState(ProcessState::Halted);
            releaseProcess(*process, false);
            if (onProcessHalted_) {
                onProcessHalted_();
            }
            process = nullptr;
            continue;
        }

        if (result == ExecutionResult::Error) {
            std::cerr << "Process " << process->id() << " error at PC="
                      << process->context().registers().pc();
            if (process->lastError()) {
                std::cerr << ": " << *process->lastError();
            }
            std::cerr << '\n';
            process->setState(ProcessState::Halted);
            process->context().setHalted(true);
            releaseProcess(*process, false);
            if (onProcessHalted_) {
                onProcessHalted_();
            }
            process = nullptr;
            continue;
        }

        if (result == ExecutionResult::Yield || process->context().shouldYield()) {
            process->context().setShouldYield(false);
            releaseProcess(*process, true);
            process = nullptr;
            continue;
        }

        if (process->context().timeSliceLeft() == 0) {
            releaseProcess(*process, true);
            process = nullptr;
        }
    }
}

ExecutionResult VirtualCpuCore::executeInstruction(ProcessControlBlock& process) {
    auto& cpu = process.context();
    auto& memory = process.memory();
    const Address pc = cpu.registers().pc();
    const Word raw = memory.readCodeWord(pc);
    const DecodedInstruction instruction = decodeInstruction(raw);

    if (tracer_) {
        tracer_->onInstruction(process.id(), pc, instruction, cpu.registers());
    }

    if (instruction.opcode == Opcode::CALL) {
        const auto syscallId = static_cast<SyscallId>(instruction.rs1);
        const Word argAddress = static_cast<Word>(static_cast<std::uint16_t>(instruction.imm));
        syscalls_.handle(syscallId, process, argAddress);
        cpu.registers().advancePc();
        if (syscallId == SyscallId::Exit) {
            return ExecutionResult::Halt;
        }
        if (syscallId == SyscallId::Yield) {
            return ExecutionResult::Yield;
        }
        if (cpu.timeSliceLeft() > 0) {
            cpu.setTimeSliceLeft(cpu.timeSliceLeft() - 1);
        }
        return ExecutionResult::Continue;
    }

    ExecutionContext ctx{cpu, memory, &process};
    const ExecutionResult result = registry_.execute(instruction, ctx);
    if (cpu.timeSliceLeft() > 0) {
        cpu.setTimeSliceLeft(cpu.timeSliceLeft() - 1);
    }
    return result;
}

void VirtualCpuCore::releaseProcess(ProcessControlBlock& process, bool preempt) {
    if (preempt && process.state() != ProcessState::Halted) {
        scheduler_.onPreempt(coreId_, process);
    } else if (process.state() != ProcessState::Halted) {
        process.setState(ProcessState::Ready);
        scheduler_.notifyProcessReady();
    } else {
        scheduler_.notifyProcessReady();
    }
}

}  // namespace vm
