#include "vm/virtual_machine.hpp"

#include "vm/debug/stdout_tracer.hpp"

namespace vm {

VirtualMachine::VirtualMachine(std::size_t coreCount, std::size_t timeSlice) : os_(coreCount, timeSlice) {}

ProcessId VirtualMachine::loadProgram(const std::string& path) {
    return os_.loadProgram(path);
}

ProcessId VirtualMachine::loadProgramFromSource(const std::string& source, const std::string& name) {
    return os_.loadProgramFromSource(source, name);
}

void VirtualMachine::run() {
    os_.run();
}

void VirtualMachine::stop() {
    os_.stop();
}

bool VirtualMachine::isRunning() const {
    return os_.isRunning();
}

ExecutionResult VirtualMachine::step(ProcessId id, std::size_t steps) {
    ExecutionResult last = ExecutionResult::Continue;
    for (std::size_t i = 0; i < steps; ++i) {
        last = os_.stepProcess(id);
        if (last == ExecutionResult::Halt || last == ExecutionResult::Error) {
            break;
        }
    }
    return last;
}

Word VirtualMachine::getRegister(ProcessId id, std::uint8_t reg) const {
    return os_.getRegister(id, reg);
}

void VirtualMachine::enableTrace(bool enabled) {
    if (enabled) {
        if (!tracer_) {
            tracer_ = std::make_shared<StdoutTracer>();
        }
        os_.setTracer(tracer_.get());
    } else {
        os_.setTracer(nullptr);
    }
}

void VirtualMachine::setTracer(std::shared_ptr<IExecutionTracer> tracer) {
    tracer_ = std::move(tracer);
    os_.setTracer(tracer_ ? tracer_.get() : nullptr);
}

}  // namespace vm
