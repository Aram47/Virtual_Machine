# VM Architecture

## Layers

```
CLI (main.cpp)
    └── VirtualMachine (facade)
            └── VirtualOS (orchestrator)
                    ├── ProgramLoader
                    ├── ProcessTable (PCB)
                    ├── PreemptiveScheduler
                    ├── SyscallHandler
                    └── VirtualCpuCore[] (std::thread)
                            ├── InstructionRegistry → IInstructionHandler[]
                            ├── IExecutionTracer (optional)
                            ├── RegisterFile / CpuContext
                            └── ProcessMemory → PhysicalMemory
```

## Execution Flow

1. ASM file → `Assembler` (2 passes) → bytecode + data
2. `VirtualOS::loadProgram` creates a `ProcessControlBlock` in `Ready` state
3. `PreemptiveScheduler` assigns the process to a free core
4. The core runs the fetch → decode → execute loop
5. On quantum expiry or `yield` — process switch on the core
6. On `halt` / `exit` — process becomes `Halted`

## Context Switch

In this educational implementation there is **no explicit register copying**: `CpuContext` lives inside the PCB and the core works with it directly. On preempt or `yield` the core releases the process; state remains in the PCB:

- GPR `r0`–`r15`
- `PC`, `SP`, flags
- `time_slice_left`

`CpuContext::saveTo` / `restoreFrom` are intended for tests and possible extensions, but are not called at runtime.

## Synchronization

- `PhysicalMemory` — `std::mutex` on every read/write
- Scheduler — `mutex` + `condition_variable` on the READY queue
- `VirtualOS::run()` — waits for all processes to finish via `condition_variable` (no polling)
- `SyscallHandler` — `mutex` on stdout for `print`
- Each core — a separate `std::thread`

## Scheduler

Round-robin with a time slice measured in instructions. On preempt the process returns to the READY queue. An idle core calls `IScheduler::onCoreIdle()` to wake the scheduler.

## Error Handling

- `ProcessMemory` validates data/stack segment bounds
- `ProcessControlBlock::lastError()` stores the execution error reason (DIV/0, stack overflow)
- Syscall errors (`scan`, `load`) throw `VmException`

## Extending the ISA

1. Add `Opcode` in `types.hpp`
2. Add the mnemonic in `OpcodeTable`
3. Implement `IInstructionHandler` and register it in `InstructionRegistry`
4. Add syntax in `Parser` (if needed)
5. Add a unit test in `instruction_test.cpp`

## Educational Limitations

- Branch offsets and addresses are 12 bits (up to 4095)
- One memory region per process (64 KB by default)
- No virtual paging
