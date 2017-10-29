    org 0x7c00  ; MBR will be loaded to memory address 0x7c00 after BIOS test.
                ; This is the first time we gain control of the computer.

[bits 16]

_start:
    jmp 0:START16  ; Ensure CS == 0

START16:
    cli  ; We don't need interrupt, we just do simple things to go into protected mode.

    xor ax, ax  ; Make all data segments (DS/ES/SS) to 0
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, _start  ; Use 0x7c00 as stack bottom, growing to lower address.
                    ; From 0x0500 to 0x7c00 there're almost 30KB free space guaranteed for our use.
                    ; We barely use any here, just a few function calls.
                    ; The bootloader (boot.c) will rely on this stack we set up.
                    ; After jump to kernel, it will set up it's own stack.
    
    call LoadBootloader
    call GetMemoryMap
    call SwitchToVesaMode
    
    ; open A20  http://wiki.osdev.org/A20
    ; this method is not perfect, but simple and reliable enough
    in al, 0x92
    or al, 2
    out 0x92, al
    
    ; Load GDT
    ; There're three descriptors: null, code and data, the later two both cover all the 4GB space.
    ; Later kernel will use it's own GDT.
    lgdt [GDTDesc]
    mov eax, cr0
    or eax, 1  ; http://wiki.osdev.org/CR0#CR0
    mov cr0, eax
    
    ; long jump to 32-bit code
    jmp (1 << 3):START32  ; 1 << 3 == 8 is the code descriptor

; Global Descriptor Table  http://wiki.osdev.org/GDT
%macro Descriptor 3  ; base, limit, attr
    dw %2 & 0xffff
    dw %1 & 0xffff
    db (%1 >> 16) & 0x00ff
    db %3 & 0xff
    db (%3 >> 8) & 0xf0 | (%2 >> 16) & 0x0f
    db %1 >> 24
%endmacro

GDT:
    Descriptor 0, 0, 0  ; null
    Descriptor 0, 0xffffffff, 0x409a  ; code
    Descriptor 0, 0xffffffff, 0x4092  ; data
GDTDesc:
    dw GDTDesc - GDT
    dd GDT

; http://www.ctyme.com/intr/rb-0708.htm
; Read [1-8) sectors (3.5KB) into memory after this MBR.
; The boot loader is bundled there, see Makefile.
LoadBootloader:
    pushad
    xor ax, ax
    mov ds, ax
    mov si, DiskAddressPacket
    mov ah, 0x42
    mov dl, 0x80
    int 0x13
    popad
    ret
; http://www.ctyme.com/intr/rb-0708.htm#Table272
DiskAddressPacket:
    db 16  ; size of this packet
    db 0  ; reserved
    dw 7  ; number of sectors
    dd 0x07e00000  ; base:offset
    dd 1  ; LBA low
    dd 0  ; LBA high

; http://www.delorie.com/djgpp/doc/rbinter/ix/10/4F.html
SwitchToVesaMode:
    pushad
    
    ;xor ax, ax
    ;mov es, ax
    ;mov di, 0xa00
    ;mov ax, 0x4f00
    ;int 0x10
    ;cmp ax, 0x004f
    ;jne Panic
    
    ;mov dx, 0x4115  ; 800x600 24bit color
    ;mov dx, 0x4118  ; 1024x768 24bit color
    ;mov dx, 0x4176  ; 1280x768 24bit color
    ;mov dx, 0x4191  ; 1920x1080 24bit color

    mov dx, 0x4143  ; 800x600 32bit color
    mov dx, 0x4177  ; 1280x768 32bit color

    ; query mode info
    mov ax, 0
    mov es, ax
    mov di, 0x600  ; 0x0600, used by kernel/graphics.cc
    mov cx, dx
    mov ax, 0x4f01
    int 0x10
    cmp ax, 0x004f
    jne Panic

    ; set SVGA video mode
    mov ax, 0x4f02
    mov bx, dx
    int 0x10
    cmp ax, 0x004f
    jne Panic
    
    popad
    ret

; http://www.uruk.org/orig-grub/mem64mb.html
GetMemoryMap:
    pushad
    
    mov ax, 0
    mov si, ax  ; range count

    mov ax, 0x70  ; 0x0700, used by kernel/memory.cc
    mov es, ax
    mov di, 8  ; 0x7008
    mov eax, 0xe820  ; query system address map
    mov ebx, 0  ; first continuation
    mov ecx, 24
    mov edx, 0x534d4150  ; 'SMAP'
GetMemoryMap_Next:
    int 0x15

    cmp eax, 0x534d4150
    jne Panic

    cmp cx, 20
    jne Panic

    cmp ebx, 0
    je GetMemoryMap_Finish

    add di, cx
    mov dword [es:0], ecx  ; save entry size
    inc si

    mov eax, 0xe820
    jnc GetMemoryMap_Next

GetMemoryMap_Finish:
    xor eax, eax
    mov ax, si
    mov dword [es:4], eax  ; save entry count
    popad
    ret

Panic:
    mov ax, 0xb800
    mov ds, ax
    mov dword [ds:0], 0x0f45
    hlt

[bits 32]
START32:
    ; init data segments
    mov ax, (2 << 3)
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax
    mov esp, _start  ; setup stack
    jmp (1 << 3):MBREnd  ; jump to boot.c

    ; Fill up MBR and sign the signature
    times 510 - ($ - $$) db 0
    dw 0xaa55

MBREnd:
