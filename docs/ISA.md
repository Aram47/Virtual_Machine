# ISA — Instruction Set Architecture

## Instruction Format (32 bits)

```
[ opcode:8 | rd:4 | rs1:4 | rs2:4 | imm:12 ]
```

- `rd` — destination register
- `rs1`, `rs2` — source registers
- `imm` — 12-bit signed immediate value or offset
- `rs2 == 0x0F` — immediate operand marker; value is in `imm`

## Registers

- `r0` … `r15` — general purpose
- Special (in `RegisterFile`): `PC`, `SP`, flags `ZF`, `SF`, `CF`, `OF`

## ASM Sections

```asm
DATA:
    BYTE  flag 1
    WORD  counter 0
CODE:
    mov r0, 10
    halt
```

Data types: `BYTE` (1), `DBYTE` (2), `WORD` (4), `QWORD` (8).

## Opcodes

| Mnemonic | Description |
|----------|-------------|
| `mov` | Copy register/immediate/memory |
| `add`, `sub`, `mul`, `div`, `mod` | Arithmetic |
| `inc`, `dec`, `not` | Unary |
| `and`, `or`, `xor` | Bitwise |
| `cmp` | Compare, update flags |
| `jmp`, `je`, `jne`, `jg`, `jge`, `jl`, `jle` | Branches |
| `load`, `store` | Data segment memory |
| `push`, `pop`, `ret` | Stack (SP grows downward) |
| `call` | System call |
| `halt` | Stop the process |

## Examples

```asm
mov r0, 5          ; immediate
mov r1, r0         ; register
mov r2, [counter]  ; load from data
store r0, [result]
cmp r0, r1
jl loop
call print, [result]
call yield
call exit, [result]
```

## Stack

SP is initialized to the top of the stack segment. The stack grows downward.

```asm
push r0           ; push register
push 42           ; push immediate
push my_label     ; push code address
pop r1            ; pop to register
ret               ; return (pop PC)
```

Recursion example: `examples/subroutine.asm` (factorial via `push`/`ret`).

## Tracing

The `--trace` CLI flag prints every executed instruction to stderr:

```
./build/vm examples/hello.asm --trace
```

## System Calls (CALL)

Encoding: `rs1 = syscall_id`, `imm = data_offset`

| ID | Name | Argument |
|----|------|----------|
| 0 | print | WORD offset |
| 1 | scan | WORD offset |
| 2 | exit | — |
| 3 | yield | — |
| 4 | load | ASCIIZ path in data |

## Flags After CMP/SUB

- `ZF` — result is zero
- `SF` — result is negative
- `CF` — borrow on subtraction
- `OF` — signed overflow
