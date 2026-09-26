; =============================================
; = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
; =============================================
; = TRS Kernel Entry Point ------------------ =
; = CREATE STACK AND WAAAAAAAIT ------------- =
; =============================================

[BITS 64]
[GLOBAL _start]
[EXTERN kmain]
[extern __bss_start]
[extern __bss_end]


[section .text.entry]
_start:
    xor ebp, ebp
    mov rdx, rdi
    
    cld
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
    jmp near hang