global isr_timer_asm

extern clock_counter;
extern process_schedule;

isr_timer_asm:
    pushad

    mov eax, [clock_counter]
    inc eax
    mov [clock_counter], eax
    
    call process_schedule
    cmp eax, 0
    je FinishSwitch

    mov ebx, [eax]  ; running_proc->pgdir
    sub ebx, 0xc0000000
    mov cr3, ebx
    mov ebx, [eax + 4]  ; running_proc->esp
    mov esp, ebx

FinishSwitch:
    mov al, 0x20
    out 0x20, al

    popad
    iret
