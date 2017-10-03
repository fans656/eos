#include "process.h"
#include "memory.h"
#include "stdio.h"
#include "elf.h"
#include "util.h"
#include "filesystem.h"
#include "math.h"
#include "time.h"
#include "string.h"
#include "array.h"

#define MAX_N_PROCS 512
#define TIMESLICE_FACTOR 5
#define CODE_SELECTOR (1 << 3)

typedef struct _Process {
    uint* pgdir;  // pgdir & esp must be first two and order is important
    uint esp;     // they're used in isr_timer.asm

    uint pid;
    const char* path;
    uint entry;
} _Process;

typedef _Process* Process;

Array ready_procs;
Process running_proc;
uint pid_alloc = 0;

extern clock_t clock_counter;

static inline void map_elf(Process proc, ELFHeader* elf) {
    ProgramHeader* ph_beg = (ProgramHeader*)((char*)elf + elf->phoff);
    for (ProgramHeader* ph = ph_beg; ph - ph_beg < elf->phnum; ++ph) { 
        if (ph->type == 1) {
            uint vbeg = ph->vaddr;
            uint vend = vbeg + ph->memsz;
            map_pages(proc->pgdir, vbeg, ph->memsz);
        }
    }
}

static inline void map_stack(Process proc) {
    map_pages(proc->pgdir, USER_STACK_BEG, STACK_SIZE);
}

static inline void load_elf(FILE* fp, ELFHeader* elf) {
    ProgramHeader* ph_beg = (ProgramHeader*)((char*)elf + elf->phoff);
    for (ProgramHeader* ph = ph_beg; ph - ph_beg < elf->phnum; ++ph) { 
        if (ph->type == 1) {
            fseek(fp, ph->offset);
            fread(fp, ph->filesz, (void*)ph->vaddr);
            if (ph->memsz > ph->filesz) {
                memset((uchar*)ph->vaddr + ph->filesz, 0, ph->memsz - ph->filesz);
            }
        }
    }
}

// https://en.wikibooks.org/wiki/X86_Assembly/Other_Instructions
static inline void prepare_stack(Process proc) {
    uint* esp = (uint*)USER_STACK_END;

    // fake the env like we just enter isr_pit_timer
    *(--esp) = 0x0202; // EFLAGS
    *(--esp) = CODE_SELECTOR;  // CS
    *(--esp) = proc->entry;  // EIP

    *(--esp) = 0;  // EAX
    *(--esp) = 0;  // ECX
    *(--esp) = 0;  // EDX
    *(--esp) = 0;  // EBX
    *(--esp) = 0;  // ESP
    *(--esp) = USER_STACK_END - 16;  // EBP
    *(--esp) = 0;  // ESI
    *(--esp) = 0;  // EDI

    proc->esp = (uint)esp;
}

void proc_new(const char* path) {
    //asm("cli");
    Process proc = named_malloc(sizeof(_Process), "Process");

    proc->pid = pid_alloc++;
    proc->path = path;
    
    // init pgdir
    proc->pgdir = alloc_frame();
    for (int i = 0; i < V2IPDE(KERNEL_BASE); ++i) {
        proc->pgdir[i] = 0;
    }
    for (int i = V2IPDE(KERNEL_BASE); i < N_PDE; ++i) {
        proc->pgdir[i] = kernel_pgdir[i];
    }
    
    // load elf content & setup stack
    FILE* fp = fopen(proc->path);
    ELFHeader* elf = alloc_frame();
    fread(fp, min(PAGE_SIZE, fsize(fp)), elf);
    proc->entry = elf->entry;

    map_elf(proc, elf);
    map_stack(proc);

    reload_cr3(proc->pgdir);
    load_elf(fp, elf);
    prepare_stack(proc);
    reload_cr3(kernel_pgdir);

    fclose(fp);
    free_frame((uint)elf);
    
    array_append(ready_procs, proc);
}

void process_exit(int status) {
    asm("cli");
    Process proc = running_proc;
    unmap_pages(proc->pgdir, 0, KERNEL_BASE);
    reload_cr3(kernel_pgdir);
    free(proc);
    running_proc = 0;
    asm("sti");
    hlt_forever();
}

uint process_schedule() {
    if (clock_counter % TIMESLICE_FACTOR) {
        return 0;
    }
    if (running_proc) {
        uint ebp;
        asm volatile("mov %0, ebp;" : "=r"(ebp));
        running_proc->esp = ebp + 8;
        array_append(ready_procs, running_proc);
        running_proc = 0;
    }
    if (array_empty(ready_procs)) {
        panic("no process");
    }
    running_proc = array_popleft(ready_procs);
    return (uint)running_proc;
}

void init_process() {
    ready_procs = array_new(MAX_N_PROCS);
    proc_new("/pa");
    proc_new("/pb");
}
