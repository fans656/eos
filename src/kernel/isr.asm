global isr_timer_asm
global isr_syscall_asm

extern clock_counter
extern running_proc
extern kernel_end
extern str_changed

extern process_schedule
extern process_exit
extern dispatch_syscall
extern _printf


isr_timer_asm:
    pushad

    mov eax, [clock_counter]
    and eax, 0x07  ; schedule every 8 times (80ms)
    cmp eax, 0
    jne isr_timer_asm_Finish
    call process_schedule
    cmp eax, 0
    je isr_timer_asm_Finish

    mov ebx, [eax]  ; running_proc->pgdir
    sub ebx, 0xc0000000
    mov cr3, ebx
    mov ebx, [eax + 4]  ; running_proc->esp
    mov esp, ebx

isr_timer_asm_Finish:
    mov eax, [clock_counter]
    inc eax
    mov [clock_counter], eax
    
    mov al, 0x20
    out 0x20, al

    popad
    iret


isr_syscall_asm:
    pushad
    
    cmp eax, 0  ; callnum == 0 for SYSCALL_EXIT
    je isr_syscall_asm_ProcExit
    
isr_syscall_asm_Dispatch:
    mov ebx, ebp
    add ebx, 8
    push ebx  ; parg
    push eax  ; callnum
    call dispatch_syscall
    add esp, 8
    mov [esp + 28], eax
    jmp isr_syscall_asm_Finish

isr_syscall_asm_ProcExit:
    mov esp, kernel_end
    mov ebx, ebp
    mov ebx, [ebx + 8]
    push ebx
    call process_exit
    add esp, 4
    
    call process_schedule
    mov ebx, [eax]  ; running_proc->pgdir
    sub ebx, 0xc0000000
    mov cr3, ebx
    mov ebx, [eax + 4]  ; running_proc->esp
    mov esp, ebx
    
isr_syscall_asm_Finish:
    popad
    iret
