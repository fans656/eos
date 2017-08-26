%include "util.inc"

    org 7c00h
    
[bits 16]
    ; fill up GDT base offset
    xor eax, eax
    mov ax, cs
    shl eax, 4
    add eax, GDT
    mov dword [GDT + 2], eax
    
    ; fill up code descriptor base offset
    xor eax, eax
    mov ax, cs
    shl eax, 4
    add eax, CODE_SEGMENT
    mov word [CODE_DESCRIPTOR + 2], ax
    shr eax, 16
    mov byte [CODE_DESCRIPTOR + 4], al
    mov byte [CODE_DESCRIPTOR + 7], ah

    lgdt [GDT]

    ; open a20
    cli
    in al, 92h
    or al, 10b
    out 92h, al

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp dword CODE_SELECTOR:0

[bits 32]
CODE_SEGMENT:
    mov ax, VIDEO_SELECTOR
    mov ds, ax
    mov edi, (12 * 80 + 40) * 2
    mov ah, 0fh
    mov al, 'f'
    mov [ds:edi], ax
    jmp $
CodeLength equ $ - CODE_SEGMENT

GDT:
    dw GDT_END - GDT - 1
    dd __
    dw NULL
CODE_DESCRIPTOR:
    Descriptor __, CodeLength, DA_OS_CODE
    CODE_SELECTOR equ CODE_DESCRIPTOR - GDT
VIDEO_DESCRIPTOR:
    Descriptor 0b8000h, 0ffffh, DA_OS_DATA
    VIDEO_SELECTOR equ VIDEO_DESCRIPTOR - GDT
GDT_END:

    times 510 - ($ - $$) db 0
    dw 0aa55h
