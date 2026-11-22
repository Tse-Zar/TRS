[BITS 64]
[GLOBAL tramp]

[section .text]
tramp:
    cli

    mov r10, rdi
    mov r11, rsi
    mov r12, rdx
    mov r13, rcx
    mov r14, r8

    mov cr3, r10

    mov rsp, r14
    and rsp, -16
    xor rbp, rbp

    mov rdi, r12
    mov rsi, r13

    jmp r11
