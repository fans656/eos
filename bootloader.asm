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
    ;mov ax, DATA_SELECTOR
    ;mov es, ax
    ;mov dword [es:0], 00102h

    ;mov ax, VIDEO_SELECTOR
    ;mov ds, ax
    ;mov edi, (12 * 80 + 40) * 2

    ;mov ax, [es:0]
    ;mov ah, 0fh
    ;mov [ds:edi], ax
    ;
    ;add edi, 2
    ;mov ax, [es:1]
    ;mov ah, 0fh
    ;mov [ds:edi], ax

    push dword MessageOffset
    call Printf
    add esp, 4

    push dword Message2Offset
    call Printf
    add esp, 4

    jmp $

Printf:
    mov ax, cs
    mov ds, ax
    mov esi, [esp + 4]
    
    mov ax, VIDEO_SELECTOR
    mov es, ax
    mov edi, 0

    ;pop edi
    mov ah, 0fh
    cld
Printf_show_char:
    lodsb
    mov [es:edi], ax
    add edi, 2
    cmp al, 0
    jne Printf_show_char
    ret

Message:
    db "hello world", 0
MessageOffset equ Message - CODE_SEGMENT
Message2:
    db "is this a test?", 0
Message2Offset equ Message2 - CODE_SEGMENT

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
DATA_DESCRIPTOR:
    Descriptor 0100000h, 2, DA_OS_DATA
    DATA_SELECTOR equ DATA_DESCRIPTOR - GDT
GDT_END:

    times 510 - ($ - $$) db 0
    dw 0aa55h
