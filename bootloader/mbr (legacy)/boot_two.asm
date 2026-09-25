; =============================================
; = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
; =============================================
; = Boot stage 2 ---------------------------- =
; = turn on A20 & read kernel & set gdt ----- =
; = Turn on safety mod & go to KERNEL ------- =
; =============================================

[BITS 16]
[ORG 0x7E00]

; --- CONST ---
KERNEL_LBA      equ 65
KERNEL_SECTORS  equ 256
LOAD_SEG        equ 0x1000          ; buffer segment (linear 0x10000)
LOAD_OFF        equ 0x0000          ; buffer offset
KERNEL_ADDR     equ 0x100000

; --- input pointer ---
start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov [boot_drive], dl

    mov si, msg_stage2
    call print_string

    ; --- Step 1: A20 ---
    mov si, msg_a20
    call print_string
    call enable_a20
    call check_a20

    test ax, ax
    jnz a20_done
    mov si, msg_a20_fail
    call print_string
de_hang:
    cli
    hlt
    jmp de_hang

a20_done:
    ; --- Step 2: read kernel (chunks of max 127 sectors) ---
    mov si, msg_load
    call print_string

    mov ecx, KERNEL_SECTORS          ; remaining sectors
    mov eax, KERNEL_LBA              ; current LBA

.read_loop:
    ; how many sectors this iteration (max 127)
    mov ebx, ecx
    cmp ebx, 127
    jbe .calc_sectors
    mov ebx, 127
.calc_sectors:
    mov [dap_sectors], bx

    ; set LBA (low 32 = eax, high 32 = 0)
    mov [dap_lba], eax
    mov dword [dap_lba + 4], 0

    ; save loop state across BIOS call
    push ecx
    push eax
    push ebx

    mov ah, 0x42
    mov dl, [boot_drive]
    mov si, dap
    int 0x13

    pop ebx
    pop eax
    pop ecx

    jc disk_error

    ; update remaining
    sub ecx, ebx
    jz .read_done

    ; advance LBA
    add eax, ebx

    ; advance buffer segment (sectors * 32 paragraphs)
    shl ebx, 5
    add [dap_buf_seg], bx

    jmp .read_loop

.read_done:
    mov si, msg_ok
    call print_string

    ; --- Step 3: GDT ---
    lgdt [gdt_desc]

    ; --- Step 4: safety mode ---
    cli
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:pm_entry

; ===========================================
; = 16-bits underprograms ----------------- =
; ===========================================

print_string:
    pusha
ps_loop:
    lodsb
    test al, al
    jz done
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    jmp ps_loop
done:
    popa
    ret

enable_a20:
    in al, 0x92
    test al, 0x02
    jnz ea20_done
    or al, 0x02
    and al, 0xFE
    out 0x92, al

ea20_done:
    ret

check_a20:
    push es
    push bx

    xor ax, ax
    mov es, ax
    mov al, [es:0x0000]
    mov [a20_save0], al
    
    mov bx, 0xFFFF
    mov es, bx
    mov al, [es:0x0010]
    mov [a20_save1], al

    xor ax, ax
    mov es, ax
    mov byte [es:0x0000], 0xAA
    mov bx, 0xFFFF
    mov es, bx
    mov byte [es:0x0010], 0x55

    xor ax, ax
    mov es, ax
    cmp byte [es:0x0000], 0xAA
    je a20_ok2
    mov ax, 0
    jmp a20_restore
a20_ok2:
    mov ax, 1
a20_restore:
    mov bl, [a20_save0]
    xor cx, cx
    mov es, cx
    mov [es:0x0000], bl
    mov bx, 0xFFFF
    mov es, bx
    mov bl, [a20_save1]
    mov [es:0x0010], bl

    pop bx
    pop es
    ret

; --- error disk ---
disk_error:
    mov si, msg_error
    call print_string
    jmp de_hang

; ===============================
; = DATA ---------------------- =
; ===============================

boot_drive      db 0
a20_save0       db 0
a20_save1       db 0

msg_stage2      db 'TRS: Stage2 started', 13, 10, 0
msg_a20         db 'TRS: Enable A20...', 13, 10, 0
msg_a20_fail    db 'TRS: A20 ERROR', 13, 10, 0
msg_load        db 'TRS: Loading kernel...', 13, 10, 0
msg_ok          db 'OK', 13, 10, 0
msg_error       db 'TRS: Kernel read error!', 13, 10, 0

; --- Disk Address Packet (with named fields) ---
dap:
    db 0x10                     ; size = 16
    db 0                        ; reserved
dap_sectors: dw 0               ; sectors to read (set by loop)
dap_buf_off:  dw LOAD_OFF       ; buffer offset (0x0000)
dap_buf_seg:  dw LOAD_SEG       ; buffer segment (0x1000 → linear 0x10000)
dap_lba:      dq 0              ; starting LBA (set by loop)

; =============================
; = GDT --------------------- =
; =============================

gdt_start:
    dq 0

gdt_core:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x9A
    db 0xCF
    db 0x00

gdt_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x92
    db 0xCF
    db 0x00

gdt_end:

gdt_desc:
    dw gdt_end - gdt_start - 1
    dd gdt_start

[BITS 32]

pm_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, 0x90000

    cld
    mov esi, 0x10000             ; было 0x8000 — теперь 0x10000
    mov edi, KERNEL_ADDR
    mov ecx, KERNEL_SECTORS * 512 / 4
    rep movsd

    jmp KERNEL_ADDR