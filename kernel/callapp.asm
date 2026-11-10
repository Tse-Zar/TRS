[BITS 64]
[GLOBAL callapp]

[section .text]
callapp:
    ; rdi = entry, rsi = stack_top : app.c::start_process
    push rbx
    mov rbx, rsp
    xor ebp, ebp
    mov rsp, rsi
    and rsi, -16
    call rdi
    mov rsp, rbx
    pop rbx
    ret

[section .note.GNU-stack noalloc noexec nowrite progbits]