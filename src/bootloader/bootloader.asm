%include "util.inc"

    org 7c00h
    
[bits 16]
    ; load kernel
    mov cx, 10
    mov ah, 0x02  ; read function
    mov bx, 0x800
    mov es, bx  ; buffer
    mov al, 64  ; number of sectors, 64 sectors = 32K max sized kernel
    mov dl, 80h  ; 1st hard disk
    mov ch, 0  ; cylinder
    mov dh, 0  ; head
    mov cl, 2  ; sector
    mov bx, 0
    int 13h
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
    jmp dword 8:0

GDT:
    dw GDT_END - GDT - 1
    dd __
    dw NULL
    Descriptor 0x8000, 0ffffffffh, DA_OS_CODE
GDT_END:

    times 510 - ($ - $$) db 0
    dw 0aa55h
