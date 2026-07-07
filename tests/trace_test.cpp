#include <gtest/gtest.h>

#include "vm/debug/i_execution_tracer.hpp"
#include "vm/virtual_machine.hpp"

#include <vector>

using namespace vm;

namespace {

class CollectingTracer : public IExecutionTracer {
public:
    void onInstruction(ProcessId pid, Address pc, const DecodedInstruction& insn,
                       const RegisterFile& regs) override {
        events_.push_back({pid, pc, insn.opcode, regs.get(0)});
    }

    struct Event {
        ProcessId pid;
        Address pc;
        Opcode opcode;
        Word r0;
    };

    const std::vector<Event>& events() const { return events_; }

private:
    std::vector<Event> events_;
};

}  // namespace

TEST(TraceTest, StepEmitsInstructionEvents) {
    VirtualMachine vm(1, 100);
    auto tracer = std::make_shared<CollectingTracer>();
    vm.setTracer(tracer);

    const ProcessId id = vm.loadProgramFromSource(R"(
CODE:
    mov r0, 9
    halt
)", "trace");

    vm.step(id, 1);
    ASSERT_GE(tracer->events().size(), 1u);
    EXPECT_EQ(tracer->events()[0].pid, id);
    EXPECT_EQ(tracer->events()[0].pc, 0u);
    EXPECT_EQ(tracer->events()[0].opcode, Opcode::MOV);
}

TEST(TraceTest, EnableTraceUsesStdoutTracer) {
    VirtualMachine vm(1, 100);
    vm.enableTrace(true);
    const ProcessId id = vm.loadProgramFromSource(R"(
CODE:
    mov r0, 1
    halt
)", "trace");
    EXPECT_EQ(vm.step(id, 1), ExecutionResult::Continue);
}
