; ============================================
; = TSEZAR TSEZAR TSEZAR TSEAR TSEZAR TSEZAR =
; ============================================
; = First stage of TRS boot. 16-bits mode    =
; = Read stage2 and check errors&bugs ------ =
; ============================================

[BITS 16]
[ORG 0x7C00]

; --- Const ---
STAGE2_SEG      equ 0x0000
STAGE2_OFF      equ 0x7E00
STAGE2_LBA      equ 1
STAGE2_SECTORS  equ 64

; --- input pointer ---
start:
    cli             ; block stack settings
    xor ax, ax      ; now ax = 0
    mov ds, ax      ; ds = 0
    mov es, ax      ; es = 0
    mov ss, ax      ; ss = 0
    mov sp, 0x7C00  ; stack (@_@)
    sti

    ; --- load disk number ---
    mov [boot_drive], dl

    ; --- booting message print ---
    mov si, msg_booting
    call print_string

    ; --- read stage2 ---
    mov ah, 0x42            ; INT 13h, ah = 42h - lba read
    mov dl, [boot_drive]    ; number of boot disk
    mov si, dap             ; disk addres packer
    int 0x13
    jc disk_error        

    mov dl, [boot_drive]
    mov si, msg_stage2
    call print_string
    jmp STAGE2_SEG:STAGE2_OFF

; --- functions ---

print_string:
    pusha
.loop:
    lodsb
    test al, al
    jz .done

    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    jmp .loop

.done:
    popa
    ret

disk_error:
    mov si, msg_error
    call print_string

    mov ah, 0x0E
    mov al, [boot_drive]
    add al, '0'
    int 0x10
.hang:
    cli
    hlt
    jmp .hang

; --- DATA ---

boot_drive  db 0
msg_booting db 'TRS: Stage1 loading...', 13, 10, 0
msg_stage2  db 'TRS: Stage1 OK, jumping to Stage2', 13, 10, 0
msg_error   db 'TRS: Disk read error!', 0

; --- DAP ---
dap:
    db 0x10
    db 0
dap_sectors:
    dw STAGE2_SECTORS
dap_buffer:
    dw STAGE2_OFF
    dw STAGE2_SEG
    dq STAGE2_LBA
dap_sectors_read:
    dw 0

; --- mbr ---
times 510-($-$$) db 0
dw 0xAA55