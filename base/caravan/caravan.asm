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
    call main
    ret

[section .note.GNU-stack noalloc noexec nowrite progbits]