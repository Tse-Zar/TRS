; =============================================
; = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
; =============================================
; = TRS Kernel Entry Point ------------------ =
; = CREATE STACK AND WAAAAAAAIT ------------- =
; =============================================

[BITS 32]
[GLOBAL _start]
[EXTERN kmain]

section .text.entry
_start:
    mov esp, 0x90000
    mov ebp, esp

    cld

    extern __bss_start
    extern __bss_end
    mov edi, __bss_start
    mov ecx, __bss_end
    sub ecx, edi
    xor eax, eax
    rep stosb

    call kmain

hang:
    cli
    hlt
    jmp hang