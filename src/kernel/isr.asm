global isr_timer_asm
global isr_syscall_asm

extern current_ticks
extern running_proc
extern kernel_end
extern current_esp

extern process_schedule
extern process_exit
extern dispatch_syscall
extern clock_tick


isr_timer_asm:
    pushad
    
    mov eax, esp
    mov [current_esp], eax

    call process_schedule
    cmp eax, 0
    je isr_timer_asm_Finish

    mov ebx, [eax]  ; running_proc->pgdir
    sub ebx, 0xc0000000
    mov cr3, ebx
    mov ebx, [eax + 4]  ; running_proc->esp
    mov esp, ebx

isr_timer_asm_Finish:
    call clock_tick
    
    mov al, 0x20  ; EOI
    out 0x20, al

    popad
    iret


isr_syscall_asm:
    pushad
    
    mov ebx, esp
    mov [current_esp], ebx
    
isr_syscall_asm_Dispatch:
    mov ebx, ebp
    add ebx, 8
    push 0
    push ebx  ; parg
    push eax  ; callnum
    call dispatch_syscall
    add esp, 8
    pop ebx
    mov [esp + 28], eax
    cmp ebx, 0
    jne isr_syscall_asm_Schedule
    jmp isr_syscall_asm_Finish
    
isr_syscall_asm_Schedule:
    call process_schedule
    cmp eax, 0
    je isr_syscall_asm_Finish
    mov ebx, [eax]  ; running_proc->pgdir
    sub ebx, 0xc0000000
    mov cr3, ebx
    mov ebx, [eax + 4]  ; running_proc->esp
    mov esp, ebx
    
isr_syscall_asm_Finish:
    popad
    iret
