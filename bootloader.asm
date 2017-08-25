    org 7c00h
    
    jmp short Start
Msg:
    db "hello eos!"
EndMsg:
Start:
    mov bx, 00fh
    mov cx, 1
    xor dx, dx
    mov ds, dx
    cld

Print:
    mov si, Msg

Char:
    mov ah, 2
    int 10h
    lodsb
    
    mov ah, 9
    int 10h
    inc dl

    cmp dl, 80
    jne Skip
    xor dl, dl
    inc dh
    
    cmp dh, 25
    jne Skip
    xor dh, dh

Skip:
    cmp si, EndMsg
    jne Char
    jmp Print

times 0200h - 2 - ($ - $$) db 0
    dw 0aa55h
