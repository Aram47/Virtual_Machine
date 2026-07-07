DATA:
    WORD message 42
CODE:
    mov r0, 5
    add r0, 10
    store r0, [message]
    call print, [message]
    halt
