%include "util.inc"

    org 7c00h
    
[bits 16]
START:
    ; video mode 13h (320x200 256 color)
    mov ax, 0x13
    int 0x10
    ; test LBA addressing
    mov ah, 0x41
    mov bx, 0x55aa
    mov dl, 0x80
    int 0x13
    jc Error
    ; load kernel
    mov ax, 0
    mov ds, ax
    mov si, DAP
    mov ah, 0x42
    mov dl, 0x80
    int 0x13
    jc Error
    ; fill up GDT base offset
    xor eax, eax
    mov ax, cs
    shl eax, 4
    add eax, GDT
    mov dword [GDT + 2], eax
    lgdt [GDT]
    ; open a20
    cli
    in al, 92h
    or al, 10b
    out 92h, al
    ; enter protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    mov ax, 16
    mov ds, ax
    jmp dword 8:0x8000

Error:
    mov ax, 0xb800
    mov ds, ax
    mov word [ds:0], 0x0f41
    jmp $

GDT:
    dw GDT_END - GDT - 1
    dd __
    dw NULL
    Descriptor 0x0000, 0ffffffffh, DA_OS_CODE
    Descriptor 0x0000, 0ffffffffh, DA_OS_DATA
GDT_END:

    times 512 - 2 - 16 - ($ - $$) db 0

; this Disk Address Packet structure must be placed at the end
; cause the build script is hard coded to rewrite the number of sectors word
DAP: ; 
    db 16  ; size of packet
    db 0  ; reserved
DAP_NUM_SECTORS:
    dw __  ; number of sectors, write by build script after determined the kernel size
DAP_BUFFER:
    dd 0x08000000  ; base:offset
DAP_LBA_LOW:
    dd 1
DAP_LBA_HIGH:
    dd 0

    dw 0aa55h
