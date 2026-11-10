; ===========================================
; = UNIV Entry Point ---------------------
; ===========================================

[BITS 64]
[GLOBAL _start]
[EXTERN main]

[section .text.entry]
_start:
    mov dx, 0x3F8
    mov al, 'U'
    out dx, al

    xor ebp, ebp
    cld

    push qword [rsp]
    call main
    ret

[section .note.GNU-stack noalloc noexec nowrite progbits]