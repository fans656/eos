%macro Descriptor 3  ; base, limit, attr
    dw %2 & 0ffffh  ; limit 0:15 bytes
    dw %1 & 0ffffh  ; base 0:15 bytes
    db (%1 >> 16) & 0ffh  ; base 16:23 
    db %3 & 0ffh  ; access byte
    db ((%3 & 0f000h) >> 8) | ((%2 >> 16) & 0fh)  ; flags and limit 16:19
    db (%1 >> 24) & 0ffh  ; base 24:31
%endmacro

DA_OS_CODE equ 4098h
DA_OS_DATA equ 0092h
__ equ 0
NULL equ 0
