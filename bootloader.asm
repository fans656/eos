    org 7c00h

Setup:
    mov ax, 0003h
    int 10h  ; clear screen

    mov bx, 000fh  ; bh = 0 for page number 0, bl = 15 for color white
    mov cx, 1  ; repetive count 1
    xor dx, dx  ; dh = 0 for row 0, dl = 0 for col 0
    mov ds, dx  ; clear ds
    cld  ; clear direction for lodsb
    mov si, MessageBeg  ; load message

Putchar:
    mov ah, 2  ; ah = 2 to set cursor position
    int 10h
    
    mov ah, 9  ; write char
    lodsb
    int 10h
    
    inc dl  ; next col
    
    cmp si, MessageEnd
    jne Putchar
    hlt

MessageBeg:
    db "hello eos!"
MessageEnd:

    times 512 - ($ - $$) - 2 db 0
    dw 0aa55h
