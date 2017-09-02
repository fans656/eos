%include "util.inc"

    org 7c00h
    
[bits 16]
    jmp 0:START
START:
    xor ax, ax
    mov ds, ax
    mov es, ax

    mov dx, 0x0518  ; 1024x768 24bit color

    ; query mode info
    mov ax, 0x50
    mov es, ax
    mov cx, dx
    mov ax, 0x4f01
    int 0x10
    cmp ax, 0x004f
    jne Error

    ; set SVGA video mode
    mov ax, 0x50
    mov es, ax
    mov ax, 0x4f02
    mov bx, dx
    int 0x10
    cmp ax, 0x004f
    jne Error

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

    ; load girl image
    mov cx, [DAP2_N_SECTORS]
    mov ebx, [DAP2_START_SECTOR]
    mov edx, [DAP2_OFFSET]
    shr edx, 12
LOOP_LOAD_GIRL_IMAGE:
    mov dword [DAP2_START_SECTOR], ebx

    mov eax, edx
    and eax, 0xffff0000
    shl eax, 12
    mov ax, dx
    mov dword [DAP2_OFFSET], eax
    add edx, 512

    pushad  ; you're keeping variables in registers,
            ; don't let them being messed up by INT call!!!
    mov si, DAP2
    mov ah, 0x42
    mov dl, 0x80
    int 0x13
    jc Error
    popad

    inc ebx
    dec cx
    cmp cx, 0
    jne LOOP_LOAD_GIRL_IMAGE

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

    times 512 - 4 - 16 - 16 - 2 - ($ - $$) db 0

DAP2_N_SECTORS:
    dw __
DAP2: ; for the girl image
    db 16  ; size of packet
    db 0  ; reserved
    dw 1  ; number of sectors
DAP2_OFFSET:
    dd 0x20000000  ; base:offset
DAP2_START_SECTOR:
    dd __
    dd 0

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

    dw 0x0000
    dw 0aa55h
