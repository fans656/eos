/*
h/ttp://wiki.osdev.org/8259_PIC
http://wiki.osdev.org/IRQ
http://wiki.osdev.org/I_Cant_Get_Interrupts_Working

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

#define IRQ_PIT_TIMER 0
#define IRQ_KEYBOARD 1

#define IRQ_MASK_TIMER 0xfe
#define IRQ_MASK_KEYBOARD 0xfd

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

clock_t extern clock_counter;

struct __attribute__((__packed__)) {
    ushort offset1;
    ushort selector;
    uchar zero;
    uchar attr;
    ushort offset2;
} idt[256];

struct __attribute__((__packed__)) {
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
	outb(PIC1_DATA, 0x20);
	io_wait();
	outb(PIC2_DATA, 0x28);
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

// send End of Interrupt to allow successive interrupts to come
void send_eoi(uchar irq) {
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
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

    uint err, vaddr;
    asm volatile("mov eax, [ebp + 4]; mov %0, eax" : "=m"(err));
    asm volatile("mov eax, cr2; mov %0, eax" : "=m"(vaddr));
    
    if ((err & PF_P) == 0) {
        panic("isr_page_fault | err: %x, vaddr: %x | %s page not present\n",
                err, vaddr, (err & PF_W) ? "write" : "read");
    } else {
        panic("isr_page_fault | err: %x, vaddr: %x\n", err, vaddr);
    }

    asm("popad; pop eax; leave; iret");
}

// interrupt 0 service routine, the PIT timer
void isr_pit_timer() {
    asm volatile ("pushad");
    ++clock_counter;
    send_eoi(IRQ_PIT_TIMER);
    asm volatile ("popad; leave; iret");
}

// interrupt 1 service routine, the keyboard
void isr_keyboard() {
    asm volatile ("pushad");
    uchar scancode = inb(0x60);
    //update_key_states(scancode);
    send_eoi(IRQ_KEYBOARD);
    asm volatile ("popad; leave; iret");
}

// interrupt 0x80 service routine, the system call
void isr_syscall() {
    uint callnum;
    asm volatile("mov %0, eax" : "=m"(callnum));

    uint* ebp;
    asm volatile("mov %0, ebp" : "=m"(ebp));
    ebp = (uint*)*ebp;
    uint* parg = ebp + 2;
    
    switch (callnum) {
        case SYSCALL_PRINTF:
            asm volatile("mov eax, %0" :: "b"(_printf((const char**)parg)));
            break;
        case SYSCALL_MALLOC:
            asm volatile("mov eax, %0" :: "b"(malloc(*parg)));
            break;
        case SYSCALL_FREE:
            free((void*)*parg);
            break;
        case SYSCALL_FOPEN:
            asm volatile("mov eax, %0" :: "b"(fopen((char*)*parg)));
            break;
        case SYSCALL_FCLOSE:
            asm volatile("mov eax, %0" :: "b"(fclose((FILE*)*parg)));
            break;
        case SYSCALL_FREAD:
            asm volatile("mov eax, %0" :: "b"(fread(
                            (FILE*)*(parg + 3),
                            *(parg + 1) * *(parg + 2),
                            (void*)*parg
                            )));
            break;
        case SYSCALL_FWRITE:
            asm volatile("mov eax, %0" :: "b"(fwrite(
                            (FILE*)*(parg + 3),
                            (void*)*parg,
                            *(parg + 1) * *(parg + 2)
                            )));
            break;
        case SYSCALL_FSIZE:
            asm volatile("mov eax, %0" :: "b"(fsize((FILE*)*parg)));
            break;
        case SYSCALL_LOAD_FILE:
            asm volatile("mov eax, %0" :: "b"(load_file((const char*)*parg)));
            break;
        case SYSCALL_BMP_BLIT:
            bmp_blit((void*)*parg,
                    (int)*(parg + 1), (int)*(parg + 2),
                    (int)*(parg + 3), (int)*(parg + 4),
                    (int)*(parg + 5), (int)*(parg + 6));
            break;
        default:
            panic("unknown syscall: %d\n", callnum);
    }

    asm volatile ("leave; iret");
}

void remap_hardware_interrupts() {
    pic_remap();
    outb(0x21,
            IRQ_MASK_TIMER
            & IRQ_MASK_KEYBOARD
            );
    outb(0xa1, 0xff);
}

void fill_idt_entries() {
    for (int i = 0; i < 256; ++i) {
        fill_idt_entry(i, isr_default);
    }

    fill_idt_entry(0x0d, isr_gpf);
    fill_idt_entry(0x0e, isr_page_fault);

    fill_idt_entry(0x20, isr_pit_timer);
    fill_idt_entry(0x21, isr_keyboard);

    fill_idt_entry(0x80, isr_syscall);
}

void init_pit() {
    ushort val = PIT_MS_PRECISION * PIT_BASE_FREQUENCY / 1000;
    asm(
            "mov ax, %0;"
            "mov ax, 0x2e9c;"
            "out 0x40, al;"
            "rol ax, 8;"
            "out 0x40, al;" :: "m"(val)
       );
}

void init_interrupt() {
    init_pit();
    remap_hardware_interrupts();
    fill_idt_entries();
    load_idt();
    asm ("sti");
}
