; ======================================
; = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
; = ---------------------------------- =
; = Save full CPU context, call C ---- =
; = ------------------------ dispather =
; ======================================

[BITS 32]

[EXTERN isr_dispatcher]
[GLOBAL isr_stub_table]

%macro ISR_NOERR 1
isr_stub_%1:
    push dword 0
    push dword %1
    jmp isr_common
%endmacro

%macro ISR_ERR 1
isr_stub_%1:
    push dword %1
    jmp isr_common
%endmacro

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_NOERR 17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31

%assign v 32
%rep 16
isr_stub_%+v:
    push dword 0
    push dword v
    jmp isr_common
%assign v v+1
%endrep

isr_stub_unused:
    push dword 0
    push dword 0xFF
    jmp isr_common

isr_common:
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp 
    call isr_dispatcher
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iretd

isr_stub_table:
%assign v 0
%rep 48
    dd isr_stub_%+v
%assign v v+1
%endrep
%rep 208
    dd isr_stub_unused
%endrep