; =============================================
; = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
; =============================================
; = TRS Kernel Entry Point ------------------ =
; = CREATE STACK AND WAAAAAAAIT ------------- =
; =============================================

[BITS 64]
[GLOBAL _start]
[EXTERN kmain]

[section .text.entry]
_start:
    xor ebp, ebp
    mov rdx, rdi

    lea rdi, [rel __bss_start]
    lea rcx, [rel __bss_end]
    sub rcx, rdi
    xor eax, eax
    rep stosb

    mov rdi, rdx
    call kmain
hang:
    cli
    hlt
    jmp hang