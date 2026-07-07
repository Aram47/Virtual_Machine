DATA:
    WORD result 0
CODE:
    mov r0, 5
    push return_addr
    jmp factorial
return_addr:
    store r0, [result]
    call print, [result]
    halt
factorial:
    cmp r0, 1
    jle base_case
    push r0
    dec r0
    push return_from_rec
    jmp factorial
return_from_rec:
    pop r1
    mul r0, r1
    ret
base_case:
    mov r0, 1
    ret
