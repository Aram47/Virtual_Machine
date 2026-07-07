# Virtual Machine

An educational virtual machine in C++17 with an ASM-like language, bytecode, a virtual OS, and a preemptive scheduler across multiple `std::thread` cores.

## Requirements

- CMake 3.16+
- A C++17-capable compiler (GCC 9+, Clang 7+, MSVC 2019+)

## Build

### Linux / macOS

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Windows (MSVC, Developer Command Prompt or PowerShell)

```powershell
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Windows (MinGW)

```bash
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Run

### Linux / macOS

```bash
./build/vm examples/hello.asm examples/arithmetic.asm --cores 4 --quantum 100
```

### Windows (MSVC)

```powershell
.\build\Release\vm.exe examples\hello.asm examples\arithmetic.asm --cores 4 --quantum 100
```

### Windows (MinGW / Ninja without configuration)

```powershell
.\build\vm.exe examples\hello.asm --cores 4 --quantum 100
```

Options:
- `--cores` — number of virtual CPU cores (default: 4)
- `--quantum` — scheduler time slice in instructions (default: 100)
- `--trace` — instruction trace to stderr

## Tests

### Linux / macOS

```bash
cd build && ctest --output-on-failure
```

### Windows (MSVC)

```powershell
cd build
ctest --output-on-failure -C Release
```

## Sample Program

```asm
DATA:
    WORD message 0
CODE:
    mov r0, 15
    store r0, [message]
    call print, [message]
    halt
```

## Architecture

- `VirtualMachine` — facade: program loading, `run()`, `stop()`, `enableTrace()`
- `VirtualOS` — orchestrator: `ProcessTable`, `ProgramLoader`, scheduler
- `VirtualCpuCore` — fetch/decode/execute in a dedicated thread
- `InstructionRegistry` — ISA handlers via `IInstructionHandler`
- `Assembler` — two-pass assembler (DATA/CODE)
- `PhysicalMemory` — shared memory with `std::mutex` and segment bounds checks

See also: [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md), [docs/ISA.md](docs/ISA.md).

## Examples

| File | Description |
|------|-------------|
| `examples/hello.asm` | print + halt |
| `examples/arithmetic.asm` | arithmetic |
| `examples/multi_task.asm` | multiple processes |
| `examples/subroutine.asm` | factorial via stack (`push`/`ret`) |

## Syscalls

| Name | Description |
|------|-------------|
| `print` | Print a WORD from memory to stdout |
| `scan` | Read an integer from stdin into memory |
| `exit` | Terminate the process |
| `yield` | Yield the time slice to the scheduler |
| `load` | Load a program from a path stored in the data segment |

Example: `call print, [message]`
# Virtual_Machine
