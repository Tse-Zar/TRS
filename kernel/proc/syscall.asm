[BITS 64]
[GLOBAL syscall_entry]
[EXTERN syscall_dispatch]
[EXTERN isr_restore]

[extern kernel_rsp]
[extern user_rsp]

%define USER_CS     0x23
%define USER_SS     0x1B
%define KERNEL_DS   0x10

section .text:
syscall_entry:
    ; swap stack from USER to KERNEL
    mov [rel user_rsp], rsp
    mov rsp, [rel kernel_rsp]

    push qword USER_SS
    push qword [rel user_rsp]
    push r11
    push qword USER_CS
    push rcx
    push qword 0
    push qword 0x77

    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8

    xor eax, eax
    mov ax, fs
    push rax
    xor eax, eax
    mov ax, gs
    push rax
    xor eax, eax

    cld
    mov rdi, rsp
    and rsp, -16
    call syscall_dispatch
    test rax, rax
    jz .bad
    mov rsp, rax

    cmp qword [rsp + 18*8], USER_CS
    jne .iret

    pop rax
    mov gs, ax
    pop rax
    mov fs, ax

    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax
    add rsp, 16

    mov rcx, [rsp]      ; rip
    mov r11, [rsp + 16] ; rflags
    mov rsp, [rsp + 24] ; user rsp
    o64 sysret          ; in nasm haven`t sysretq =(

.iret:
    jmp isr_restore
.bad:
    ud2
