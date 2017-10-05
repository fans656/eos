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
#include "assert.h"

#define MAX_N_PROCS 512
#define MAX_COUNTDOWNS 512
#define TIMESLICE_FACTOR 5
#define CODE_SELECTOR (1 << 3)

extern clock_t clock_counter;
extern uint kernel_end;

Array ready_procs;
Array exited_procs;
Process running_proc;
uint current_esp;
uint pid_alloc = 0;

typedef struct CountDown {
    uint cnt;
    Process proc;
} CountDown;

Array countdowns;

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

Process proc_new(const char* path) {
    Process proc = named_malloc(sizeof(_Process), "Process");

    proc->path = path;
    proc->pid = pid_alloc++;
    
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
    
    return proc;
}

Process proc_idle() {
    Process proc = named_malloc(sizeof(_Process), "Process idle");
    proc->path = "idle";
    proc->pid = pid_alloc++;
    proc->pgdir = kernel_pgdir;
    proc->esp = 0;  // not relevant, the first process switch will set this
    proc->entry = 0;  // not relevant, this will be the running_proc right away
    return proc;
}

void process_exit(int status) {
    array_append(exited_procs, running_proc);
    running_proc = 0;
}

void process_release() {
    while (!array_empty(exited_procs)) {
        Process proc = array_popleft(exited_procs);
        unmap_pages(proc->pgdir, 0, KERNEL_BASE);
        free(proc);
    }
}

int g__i = 0;

uint process_schedule() {
    if (running_proc) {
        running_proc->esp = current_esp;
        array_append(ready_procs, running_proc);
        running_proc = 0;
    }
    if (array_empty(ready_procs)) {
        panic("no process");
    }
    running_proc = array_popleft(ready_procs);
    return (uint)running_proc;
}

CountDown countdown_pool[1024];
size_t countdown_pool_alloc = 0;

void process_sleep(uint ms) {
    CountDown* cd = (CountDown*)named_malloc(sizeof(CountDown), "CountDown");
    cd->cnt = (ms + PIT_MS_PRECISION - 1) / PIT_MS_PRECISION;
    cd->proc = running_proc;
    array_append(countdowns, cd);
    running_proc->esp = current_esp;
    running_proc = 0;
}

void process_count_down() {
    size_t i = 0;
    while (i < array_size(countdowns)) {
        CountDown* cd = array_get(countdowns, i);
        if (!--cd->cnt) {
            array_append(ready_procs, cd->proc);
            array_remove(countdowns, i);
            free(cd);
            continue;
        }
        ++i;
    }
}

void dump_procs() {
    printf("======================================= dump_procs\n");
    if (running_proc) {
        printf("Running: %d(%s)\n", running_proc->pid, running_proc->path);
    } else {
        printf("no running_proc\n");
    }
    printf("Ready: ");
    for (int i = 0; i < array_size(ready_procs); ++i) {
        Process proc = (Process)array_get(ready_procs, i);
        printf("%d(%s) ", proc->pid, proc->path);
    }
    putchar('\n');
    printf("Count downs: ");
    for (int i = 0; i < array_size(countdowns); ++i) {
        CountDown* cd = (CountDown*)array_get(countdowns, i);
        Process proc = cd->proc;
        printf("%d(%s) ", proc->pid, proc->path);
    }
    putchar('\n');
    printf("======================================= dump_procs end\n");
}

void init_process() {
    running_proc = proc_idle();

    countdowns = array_new(MAX_COUNTDOWNS);
    ready_procs = array_new(MAX_N_PROCS);
    exited_procs = array_new(MAX_N_PROCS);

    //array_append(ready_procs, proc_new("/bin/init"));
}
