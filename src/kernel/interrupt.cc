/*
h/ttp://wiki.osdev.org/8259_PIC
http://wiki.osdev.org/IRQ
http://wiki.osdev.org/I_Cant_Get_Interrupts_Working
http://wiki.osdev.org/PIC#Programming_the_PIC_chips

https://forum.arduino.cc/index.php?topic=65433.0
there's automatically `cli` before entering ISR and `sti` after returning from ISR
 */
#include "interrupt.h"
#include "util.h"
#include "asm.h"
#include "stdio.h"
#include "memory.h"
#include "filesystem.h"
#include "graphics.h"
#include "time.h"
#include "process.h"
#include "keyboard.h"
#include "message.h"
#include "graphics.h"
#include "mouse.h"
#include "process.h"

#define IRQ_OFFSET 0x20

#define IRQ_PIT_TIMER 0
#define IRQ_KEYBOARD 1
#define IRQ_MOUSE 12

#define IRQ_MASK_TIMER 0xfe
#define IRQ_MASK_SLAVE 0xfb
#define IRQ_MASK_KEYBOARD 0xfd
#define IRQ_MASK_MOUSE 0xef

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xa0
#define PIC2_DATA 0xa1

#define PIC_EOI 0x20
#define PIC_READ_IRR 0x0a
#define PIC_READ_ISR 0x0b

#define ICW1_ICW4 0x01
#define ICW1_SINGLE 0x02
#define ICW1_INTERVAL4 0x04
#define ICW1_LEVEL 0x08
#define ICW1_INIT 0x10
 
#define ICW4_8086 0x01
#define ICW4_AUTO 0x02
#define ICW4_BUF_SLAVE 0x08
#define ICW4_BUF_MASTER 0x0C
#define ICW4_SFNM 0x10

// send End of Interrupt to allow successive interrupts to come
void send_eoi(uchar irq) {
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}

void isr_default() {
    panic("isr_default\n");
}

void isr_gpf() {
    panic("isr_gpf\n");
}

// http://wiki.osdev.org/Exceptions#Page_Fault
#define PF_P 0x01
#define PF_W 0x02
void isr_page_fault() {
    asm("pushad");

    uint err, vaddr, eip;
    asm volatile("mov eax, [ebp + 4]; mov %0, eax" : "=m"(err));
    asm volatile("mov eax, cr2; mov %0, eax" : "=m"(vaddr));
    asm volatile("mov eax, [ebp + 8]; mov %0, eax" : "=m"(eip));
    
    if ((err & PF_P) == 0) {
        printf("PageFault err %x vaddr %x eip %x %s\n",
                err, vaddr, eip, (err & PF_W) ? "W" : "R");
    } else {
        panic("isr_page_fault | err: %x, vaddr: %x\n", err, vaddr);
    }
    
    //dump_procs();
    panic("");

    asm("popad; pop eax; leave; iret");
}

extern "C" void isr_timer_asm();
extern "C" void isr_syscall_asm();

void isr_keyboard() {
    asm volatile ("pushad");
    uchar scancode = inb(0x60);
    //update_key_state(scancode);
    send_eoi(IRQ_KEYBOARD);
    asm volatile ("popad; leave; iret");
}

uchar mouse_cycle = 0;
uchar mouse_bytes[3];
bool gui_inited = false;
GUIMouseEvent* mouse_events_pool = 0;
size_t mouse_events_pool_idx = 0;
constexpr size_t mouse_events_pool_size = 256;

void isr_mouse() {
    asm volatile ("pushad");
    uchar val = inb(0x60);
    switch (mouse_cycle) {
        case 0:
            if (val & 0x08) {
                mouse_bytes[mouse_cycle++] = val;
            }
            break;
        case 1:
            mouse_bytes[mouse_cycle++] = val;
            break;
        case 2:
            mouse_bytes[mouse_cycle] = val;
            mouse_cycle = 0;
            parse_mouse_event(mouse_bytes, mouse_events_pool[mouse_events_pool_idx]);
            if (gui_inited) {
                replace_message(GUI_MOUSE_EVENT_ID, &mouse_events_pool[mouse_events_pool_idx]);
                mouse_events_pool_idx = (mouse_events_pool_idx + 1) % mouse_events_pool_size;
            }
            break;
    }
    send_eoi(IRQ_MOUSE);
    asm volatile ("popad; leave; iret");
}

extern "C" uint dispatch_syscall(uint callnum, uint* parg, uint do_schedule) {
    switch (callnum) {
        case SYSCALL_PRINTF:
            return (uint)_printf((const char**)parg);
        case SYSCALL_EXIT:
            process_exit((int)*parg);
            do_schedule = 1;
            break;
        case SYSCALL_SLEEP:
            process_sleep((uint)*parg);
            do_schedule = 1;
            break;
        case SYSCALL_YIELD:
            do_schedule = 1;
            break;
        case SYSCALL_MALLOC:
            return (uint)malloc((size_t)*parg);
        case SYSCALL_FREE:
            free((void*)*parg);
            break;
        case SYSCALL_FOPEN:
            return (uint)fopen((const char*)*parg, (const char*)*(parg + 1));
        case SYSCALL_FCLOSE:
            return (uint)fclose((FILE*)*parg);
        case SYSCALL_FREAD:
            return (uint)fread((void*)*(parg), (size_t)*(parg + 1),
                    (size_t)*(parg + 2), (FILE*)*(parg + 3));
        case SYSCALL_FWRITE:
            return (uint)fwrite((void*)*(parg), (size_t)*(parg + 1),
                    (size_t)*(parg + 2), (FILE*)*(parg + 3));
        case SYSCALL_FSIZE:
            return (uint)fsize((FILE*)*parg);
        case SYSCALL_LOAD_FILE:
            return (uint)load_file((const char*)*parg);
        case SYSCALL_MEMORY_BLIT:
            memory_blit((uchar*)*parg, (int)*(parg + 1),
                    (int)*(parg + 2), (int)*(parg + 3),
                    (int)*(parg + 4), (int)*(parg + 5),
                    (int)*(parg + 6), (int)*(parg + 7));
            break;
        case SYSCALL_GET_MESSAGE:
            {
                bool blocking = (bool)*(parg + 1);
                void* msg = get_message((int)*parg, blocking);
                if (msg == 0 && blocking) {
                    do_schedule = 1;
                    break;
                } else {
                    return (uint)msg;
                }
            }
        case SYSCALL_PUT_MESSAGE:
            put_message((int)*parg, (void*)*(parg + 1));
            break;
        case SYSCALL_SET_TIMER:
            return (uint)set_timer((uint)*parg, (uint)*(parg + 1), (bool)*(parg + 2));
        case SYSCALL_TIMEIT:
            _timeit((const char**)parg);
            break;
        case SYSCALL_INIT_GUI:
            gui_inited = true;
            mouse_events_pool = new GUIMouseEvent[mouse_events_pool_size];
            return (uint)new GUIInfo(
                    get_screen_width(), get_screen_height(),
                    get_screen_pitch(), get_screen_bpp());
        default:
            panic("unknown syscall %d\n", callnum);
    }
    return 0;
}

void io_wait() {
    asm("nop;nop;nop;nop;");
}

// remap Programed Interrupt Controller
// to avoid the 15 hardware interrupts overlap with software exceptions
void pic_remap() {
	outb(PIC1_COMMAND, ICW1_INIT+ICW1_ICW4);
	io_wait();
	outb(PIC2_COMMAND, ICW1_INIT+ICW1_ICW4);
	io_wait();
	outb(PIC1_DATA, IRQ_OFFSET);
	io_wait();
	outb(PIC2_DATA, IRQ_OFFSET + 8);
	io_wait();
	outb(PIC1_DATA, 4);
	io_wait();
	outb(PIC2_DATA, 2);
	io_wait();
 
	outb(PIC1_DATA, ICW4_8086);
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();
}

ushort pic_get_status_register(uchar ocw3) {
    outb(PIC1_COMMAND, ocw3);
    outb(PIC2_COMMAND, ocw3);
    return (inb(PIC2_COMMAND) << 8) | inb(PIC1_COMMAND);
}

ushort pic_get_in_service_register() {
    return pic_get_status_register(PIC_READ_ISR);
}

ushort pic_get_interrupt_request_register() {
    return pic_get_status_register(PIC_READ_IRR);
}

void init_pit() {
    ushort divider = PIT_MS_PRECISION * PIT_BASE_FREQUENCY / 1000;
    if (!(0 < divider && divider <= 65535)) {
        panic("init_pit: divider out of range %d\n", divider);
    }
    asm(
            "mov ax, %0;"
            "out 0x40, al;"
            "rol ax, 8;"
            "out 0x40, al;" :: "m"(divider)
       );
}

void remap_hardware_interrupts() {
    pic_remap();
    outb(0x21, IRQ_MASK_TIMER & IRQ_MASK_KEYBOARD & IRQ_MASK_SLAVE);
    outb(0xa1, IRQ_MASK_MOUSE);
}

struct __attribute__((__packed__)) IDT {
    ushort offset1;
    ushort selector;
    uchar zero;
    uchar attr;
    ushort offset2;
} idt[256];

struct __attribute__((__packed__)) IDTR {
    ushort limit : 16;
    uint base : 32;
} idtr;

typedef void (*ISR)();

void fill_idt_entry(int idx, ISR isr) {
    uint addr = (uint)isr;
    idt[idx].selector = 8;
    idt[idx].zero = 0;
    idt[idx].attr = isr ? 0x8e : 0x00;
    idt[idx].offset1 = addr & 0xffff;
    idt[idx].offset2 = addr >> 16;
}

void load_idt() {
    idtr.limit = 256 * 8 - 1;
    idtr.base = (uint)idt;
    asm volatile ("mov eax, %0" :: "r"(&idtr));
    asm volatile ("lidt [eax]");
}

void fill_idt_entries() {
    for (int i = 0; i < 256; ++i) {
        fill_idt_entry(i, isr_default);
    }

    fill_idt_entry(0x0d, isr_gpf);
    fill_idt_entry(0x0e, isr_page_fault);

    fill_idt_entry(IRQ_OFFSET + IRQ_PIT_TIMER, isr_timer_asm);
    fill_idt_entry(IRQ_OFFSET + IRQ_KEYBOARD, isr_keyboard);
    fill_idt_entry(IRQ_OFFSET + IRQ_MOUSE, isr_mouse);

    fill_idt_entry(0x80, isr_syscall_asm);
}

void init_interrupt() {
    init_pit();
    remap_hardware_interrupts();
    fill_idt_entries();
    load_idt();
}
