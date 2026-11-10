; ===========================================
; = CARAVAN Entry Point ---------------------
; ===========================================

[BITS 64]
[GLOBAL _start]
[EXTERN main]

[section .text.entry]
_start:
    xor ebp, ebp
    cld
    and rsp, -16
    call main
    ret

[section .note.GNU-stack noalloc noexec nowrite progbits]