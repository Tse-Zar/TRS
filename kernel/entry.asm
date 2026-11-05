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
[extern __kernel_start]
[extern __kernel_end]
[extern __hdr_image_size]
[extern __hdr_mem_pages]
[extern __hdr_text_pages]
[extern __hdr_entry_off]
[section .head]
kern_hdr:
    dd 0x5452534B
    dd __hdr_image_size
    dd __hdr_mem_pages
    dd __hdr_text_pages
    dd __hdr_entry_off
    dd 64
    times 64 - ($ - kern_hdr) db 0

[section .text.entry]
_start:

    mov dx, 0x3F8
    mov al, 'E'
    out dx, al
    
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


[section .note.GNU-stack noalloc noexec nowrite progbits]