[BITS 64]
[EXTERN isr_restore]
[GLOBAL jump_to_frame]

; rdi: regs_t* frame
jump_to_frame:
    mov rsp, rdi
    jmp isr_restore

[section .note.GNU-stack noalloc noexec nowrite progbits]
