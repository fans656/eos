#include "process.h"
#include "memory.h"
#include "stdio.h"
#include "elf.h"
#include "util.h"
#include "filesystem.h"
#include "math.h"
#include "time.h"
#include "string.h"
#include "list.h"
#include "message.h"
#include "debug.h"

#define CODE_SELECTOR (1 << 3)

extern clock_t clock_counter;
extern uint kernel_end;

static inline uint ms2tick(uint ms) {
    return (ms + PIT_MS_PRECISION - 1) / PIT_MS_PRECISION;
}

struct CountDown {
    CountDown(uint ms) : cnt(ms2tick(ms)), dead_(false) {}

    virtual void count() {
        --cnt;
    }
    
    inline bool dead() const { return dead_; }
    
    uint cnt;
    bool dead_;
};

List<CountDown*> countdowns;
List<Process> ready_procs;
List<Process> blocked_procs;
List<Process> exited_procs;
List<const char*> init_procs;
Process running_proc;
uint current_esp;
uint pid_alloc = 0;

struct TimerCountDown : public CountDown {
    TimerCountDown(uint ms, uint queue_id, uint timer_id, bool singleshot = false)
        : CountDown(ms), queue_id(queue_id), timer_id(timer_id), singleshot(singleshot) {
            initial_cnt = cnt;
    }
    
    void count() {
        CountDown::count();
        if (cnt == 0) {
            if (singleshot) {
                dead_ = true;
            } else {
                cnt = initial_cnt;
            }
            put_message(queue_id, new TimerEvent(timer_id));
        }
    }

    uint initial_cnt;
    uint queue_id;
    uint timer_id;
    bool singleshot;
};

struct SleepCountDown : public CountDown {
    SleepCountDown(uint ms, Process proc) : CountDown(ms), proc(proc) {}
    
    void count() {
        CountDown::count();
        if (cnt == 0) {
            dead_ = true;
            ready_procs.append(proc);
        }
    }
    
    Process proc;
};

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
            fread((void*)ph->vaddr, ph->filesz, 1, fp);
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
    Process proc = (Process)named_malloc(sizeof(_Process), path);

    proc->path = path;
    proc->pid = pid_alloc++;
    
    // init pgdir
    proc->pgdir = (uint*)alloc_frame();
    for (int i = 0; i < V2IPDE(KERNEL_BASE); ++i) {
        proc->pgdir[i] = 0;
    }
    for (int i = V2IPDE(KERNEL_BASE); i < N_PDE; ++i) {
        proc->pgdir[i] = kernel_pgdir[i];
    }
    
    // load elf content & setup stack
    FILE* fp = fopen(proc->path, "rb");
    ELFHeader* elf = (ELFHeader*)alloc_frame();
    fread(elf, min(PAGE_SIZE, fsize(fp)), 1, fp);
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

Process proc_kernel() {
    Process proc = (Process)named_malloc(sizeof(_Process), "Process kernel");
    proc->path = "kernel";
    proc->pid = pid_alloc++;
    proc->pgdir = kernel_pgdir;
    proc->esp = 0;  // not relevant, the first process switch will set this
    proc->entry = 0;  // not relevant, this will be the running_proc right away
    return proc;
}

void process_exit(int status) {
    exited_procs.append(running_proc);
    running_proc = 0;
}

void process_release() {
    while (!exited_procs.empty()) {
        Process proc = exited_procs.popleft();
        unmap_pages(proc->pgdir, 0, KERNEL_BASE);
        delete proc;
    }
}

uint process_schedule() {
    if (running_proc) {
        running_proc->esp = current_esp;
        ready_procs.append(running_proc);
        running_proc = 0;
    }
    if (ready_procs.empty()) {
        dump_procs();
        panic("no process");
    }
    running_proc = ready_procs.popleft();

    //++debug_info.proc_switch_cnt;

    return (uint)running_proc;
}

void process_yield() {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_YIELD));
}

bool process_is_idle() {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_IS_IDLE));
}

bool _process_is_idle() {
    return ready_procs.empty();
}

void process_block() {
    blocked_procs.append(running_proc);
    running_proc->esp = current_esp;
    running_proc = 0;
}

void process_unblock(Process proc) {
    if (blocked_procs.remove(proc)) {
        ready_procs.append(proc);
    }
}

void process_sleep(uint ms) {
    auto cd = new SleepCountDown(ms, running_proc);
    countdowns.append(cd);
    running_proc->esp = current_esp;
    running_proc = 0;
}

void process_count_down() {
    auto it = countdowns.begin();
    for (auto cd: countdowns) {
        cd->count();
        if (cd->dead()) {
            delete cd;
            it.remove();
        }
        ++it;
    }
}

static uint timer_id_alloc = 0;

uint set_timer(uint ms, uint queue_id, bool singleshot) {
    uint timer_id = timer_id_alloc++;
    countdowns.append(new TimerCountDown(ms, queue_id, timer_id, singleshot));
    return timer_id;
}

void dump_procs() {
    printf("======================================= dump_procs\n");
    if (running_proc) {
        printf("Running: %d(%s)\n", running_proc->pid, running_proc->path);
    } else {
        printf("no running_proc\n");
    }
    printf("Ready: ");
    for (auto proc: ready_procs) {
        printf("%d(%s) ", proc->pid, proc->path);
    }
    putchar('\n');
    printf("======================================= dump_procs end\n");
}

void process_init(const char* fpath) {
    char* fpath_ = new char[strlen(fpath) + 1];
    strcpy(fpath_, fpath);
    init_procs.append(fpath_);
}

void process_start() {
    asm("cli");
    while (!init_procs.empty()) {
        auto fpath = init_procs.popleft();
        ready_procs.append(proc_new(fpath));
    }
    asm("sti");
}

void init_process() {
    running_proc = proc_kernel();

    countdowns.construct();
    ready_procs.construct();
    blocked_procs.construct();
    exited_procs.construct();
    init_procs.construct();

    ready_procs.append(proc_new("/bin/gui"));
    ready_procs.append(proc_new("/bin/desktop"));
    //ready_procs.append(proc_new("/bin/pa"));
    ready_procs.append(proc_new("/bin/pb"));
    //ready_procs.append(proc_new("/bin/pc"));
    ready_procs.append(proc_new("/bin/terminal"));
}
