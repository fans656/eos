%include "util.inc"

    org 7c00h
    
[bits 16]
START:
    ; fill up GDT base offset
    xor eax, eax
    mov ax, cs
    shl eax, 4
    add eax, GDT
    mov dword [GDT + 2], eax
    lgdt [GDT]
    ; load kernel
    mov cx, 10
    mov ah, 0x02  ; read function
    mov bx, 0x800
    mov es, bx  ; buffer
    mov al, 32  ; number of sectors, 64 sectors = 16K max sized kernel
    mov dl, 80h  ; 1st hard disk
    mov ch, 0  ; cylinder
    mov dh, 0  ; head
    mov cl, 2  ; sector
    mov bx, 0
    int 13h
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

GDT:
    dw GDT_END - GDT - 1
    dd __
    dw NULL
    Descriptor 0x0000, 0ffffffffh, DA_OS_CODE
    Descriptor 0x0000, 0ffffffffh, DA_OS_DATA
GDT_END:

    times 510 - ($ - $$) db 0
    dw 0aa55h
