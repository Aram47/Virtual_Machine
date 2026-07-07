#include <gtest/gtest.h>

#include "vm/virtual_machine.hpp"

#include <filesystem>
#include <fstream>

#ifndef VM_SOURCE_DIR
#define VM_SOURCE_DIR "."
#endif

using namespace vm;

namespace {

std::string examplePath(const char* filename) {
    return (std::filesystem::path(VM_SOURCE_DIR) / "examples" / filename).string();
}

}  // namespace

TEST(IntegrationTest, RunsHelloProgramToHalt) {
    VirtualMachine vm(1, 100);
    const ProcessId id = vm.loadProgram(examplePath("hello.asm"));

    for (int i = 0; i < 20; ++i) {
        const ExecutionResult result = vm.step(id, 1);
        if (result == ExecutionResult::Halt) {
            break;
        }
    }

    EXPECT_EQ(vm.getRegister(id, 0), 15u);
}

TEST(IntegrationTest, ArithmeticProgram) {
    VirtualMachine vm(1, 100);
    const ProcessId id = vm.loadProgram(examplePath("arithmetic.asm"));

    for (int i = 0; i < 30; ++i) {
        if (vm.step(id, 1) == ExecutionResult::Halt) {
            break;
        }
    }

    EXPECT_EQ(vm.getRegister(id, 2), 29u);
}

TEST(IntegrationTest, MultiCoreRunsMultiplePrograms) {
    VirtualMachine vm(2, 5);
    const ProcessId first = vm.loadProgramFromSource(R"(
CODE:
    mov r0, 1
    halt
)", "one");
    const ProcessId second = vm.loadProgramFromSource(R"(
CODE:
    mov r0, 2
    halt
)", "two");

    vm.run();

    EXPECT_EQ(vm.getRegister(first, 0), 1u);
    EXPECT_EQ(vm.getRegister(second, 0), 2u);
}

TEST(IntegrationTest, SubroutineFactorial) {
    VirtualMachine vm(1, 100);
    const ProcessId id = vm.loadProgram(examplePath("subroutine.asm"));

    for (int i = 0; i < 200; ++i) {
        if (vm.step(id, 1) == ExecutionResult::Halt) {
            break;
        }
    }

    EXPECT_EQ(vm.getRegister(id, 0), 120u);
}
