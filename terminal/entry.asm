; =============================================
; = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
; =============================================
; = TRS Terminal Entry Point ----------------- =
; = STACK IS SET UP BY start_process --------- =
; =============================================

[BITS 64]
[GLOBAL _start]
[EXTERN main]

[section .text.entry]
_start:

    mov dx, 0x3F8
    mov al, 'P'
    out dx, al

    xor ebp, ebp
    cld

    push qword [rsp]
    call main
    mov edi, eax
    ret

[section .note.GNU-stack noalloc noexec nowrite progbits]