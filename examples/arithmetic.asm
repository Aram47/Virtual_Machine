DATA:
    WORD result 0
CODE:
    mov r0, 10
    mov r1, 3
    add r2, r0
    add r2, r1
    mul r2, r1
    sub r2, r0
    store r2, [result]
    call print, [result]
    halt
