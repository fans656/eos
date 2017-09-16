%include "util.inc"

    org 7c00h
    
[bits 16]
    jmp 0:START
START:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; get memory info
    ; http://www.uruk.org/orig-grub/mem64mb.html
;    mov di, 0x7c00 - 0x0400
;    mov ebx, 0
;Next:
;    mov eax, 0xe820
;    mov ecx, 0x0400
;    mov edx, 0x534d4150
;    int 0x15
;    jc Error
;    cmp eax, 0x534d4150
;    jne Error
;    add di, cx
;    cmp ebx, 0
;    jne Next
;    sub di, 0x7c00 - 0x0400
;    mov [0x0600], di
    
    ;call SwithToVesaMode

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

    jmp dword 8:0x8000

SwithToVesaMode:
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
    
    ret

Error:
    mov ax, 0xb800
    mov ds, ax
    mov word [ds:(12 * 80 + 39) * 2], 0x0f45
    jmp $

GDT:
    dw GDT_END - GDT - 1
    dd __
    dw NULL
    Descriptor 0x0000, 0ffffffffh, DA_OS_CODE
    Descriptor 0x0000, 0ffffffffh, DA_OS_DATA
GDT_END:

    times 512 - 2 - 64 - 2 - 16 - ($ - $$) db 0

; this Disk Address Packet structure must be placed here
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

PADDING:
    dw 0x0000

PARTITION_TABLE:
    times 16 * 4 db 0

    dw 0aa55h
