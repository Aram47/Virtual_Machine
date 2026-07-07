cmake_minimum_required(VERSION 3.16)
project(virtual_machine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_library(vm_core STATIC
    src/vm/types.cpp
    src/vm/memory/physical_memory.cpp
    src/vm/memory/process_memory.cpp
    src/vm/cpu/register_file.cpp
    src/vm/cpu/cpu_context.cpp
    src/vm/cpu/virtual_cpu_core.cpp
    src/vm/isa/instruction_helpers.cpp
    src/vm/isa/opcode_table.cpp
    src/vm/isa/instruction_registry.cpp
    src/vm/isa/handlers/arithmetic_handlers.cpp
    src/vm/isa/handlers/logic_handlers.cpp
    src/vm/isa/handlers/jump_handlers.cpp
    src/vm/isa/handlers/memory_handlers.cpp
    src/vm/isa/handlers/stack_handlers.cpp
    src/vm/isa/handlers/control_handlers.cpp
    src/vm/process/process_control_block.cpp
    src/vm/process/process_table.cpp
    src/vm/os/syscall_handler.cpp
    src/vm/os/preemptive_scheduler.cpp
    src/vm/os/program_loader.cpp
    src/vm/os/virtual_os.cpp
    src/vm/debug/stdout_tracer.cpp
    src/vm/assembler/lexer.cpp
    src/vm/assembler/parser.cpp
    src/vm/assembler/assembler.cpp
    src/vm/virtual_machine.cpp
)

target_include_directories(vm_core PUBLIC include)
target_link_libraries(vm_core PUBLIC Threads::Threads)

add_executable(vm main.cpp)
target_link_libraries(vm PRIVATE vm_core)

enable_testing()

add_executable(vm_tests
    tests/memory_test.cpp
    tests/register_test.cpp
    tests/instruction_test.cpp
    tests/assembler_test.cpp
    tests/scheduler_test.cpp
    tests/syscall_test.cpp
    tests/integration_test.cpp
    tests/trace_test.cpp
)

target_link_libraries(vm_tests PRIVATE vm_core GTest::gtest_main Threads::Threads)
target_compile_definitions(vm_tests PRIVATE VM_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
include(GoogleTest)
gtest_discover_tests(vm_tests)
