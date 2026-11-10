[BITS 64]
[GLOBAL callapp]

[section .text]
callapp:
    ; rdi = entry, rsi = stack_top : app.c::start_process
    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15
    
    mov r15, rsp
    mov rsp, rsi
    and rsp, -16
    xor ebp, ebp
    call rdi

    mov rsp, r15
    
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    pop rbx
    ret

[section .note.GNU-stack noalloc noexec nowrite progbits]