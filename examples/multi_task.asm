DATA:
    WORD counter 0
    WORD limit 3
CODE:
loop:
    call print, [counter]
    mov r0, [counter]
    inc r0
    store r0, [counter]
    mov r1, [limit]
    cmp r0, r1
    jl loop
    call yield
    mov r0, 0
    store r0, [counter]
    mov r1, [limit]
    cmp r0, r1
    jl loop
    halt
