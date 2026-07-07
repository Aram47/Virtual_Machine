#pragma once

#include "vm/debug/i_execution_tracer.hpp"
#include "vm/os/virtual_os.hpp"

#include <memory>

namespace vm {

class VirtualMachine {
public:
    explicit VirtualMachine(std::size_t coreCount = kDefaultCoreCount,
                            std::size_t timeSlice = kDefaultTimeSlice);

    ProcessId loadProgram(const std::string& path);
    ProcessId loadProgramFromSource(const std::string& source, const std::string& name);

    void run();
    void stop();
    bool isRunning() const;

    void enableTrace(bool enabled);
    void setTracer(std::shared_ptr<IExecutionTracer> tracer);

    ExecutionResult step(ProcessId id, std::size_t steps = 1);
    Word getRegister(ProcessId id, std::uint8_t reg) const;

private:
    VirtualOS os_;
    std::shared_ptr<IExecutionTracer> tracer_;
};

}  // namespace vm
