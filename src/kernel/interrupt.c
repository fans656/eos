/*
http://wiki.osdev.org/8259_PIC
http://wiki.osdev.org/IRQ
http://wiki.osdev.org/I_Cant_Get_Interrupts_Working
 */
#include <stdint.h>
#include <stdbool.h>
#include "interrupt.h"
#include "io.h"
#include "util.h"
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

IDTEntry idt[256];

typedef struct __attribute__((__packed__)) {
    uint16_t limit : 16;
    uint32_t base : 32;
} IDTR;

typedef void (*ISR)();

void fill_idt_entry(int idx, ISR isr) {
    uint32_t addr = (uint32_t)isr;
    idt[idx].offset1 = addr & 0x0000ffff;
    idt[idx].offset2 = addr & 0xffff0000;
    idt[idx].selector = 8;
    idt[idx].zero = 0;
    idt[idx].attr = isr ? 0x8e : 0x00;
}

void io_wait() {
    asm("nop;nop;nop;nop;");
}

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

void send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}

uint16_t pic_get_status_register(uint8_t ocw3) {
    outb(PIC1_COMMAND, ocw3);
    outb(PIC2_COMMAND, ocw3);
    return (inb(PIC2_COMMAND) << 8) | inb(PIC1_COMMAND);
}

uint16_t pic_get_in_service_register() {
    return pic_get_status_register(PIC_READ_ISR);
}

uint16_t pic_get_interrupt_request_register() {
    return pic_get_status_register(PIC_READ_IRR);
}

void isr_pit_timer() {
    asm volatile ("pushad");
    
    ++g_clock_counter;
    
    if (sleep_count >= 0) {
        --sleep_count;
    }

    send_eoi(IRQ_PIT_TIMER);
    asm volatile ("popad; leave; iret");
}

void isr_keyboard() {
    asm volatile ("pushad");
    update_key_states(inb(0x60));
    send_eoi(IRQ_KEYBOARD);
    asm volatile ("popad; leave; iret");
}

void setup_idt() {
    IDTR idtr;
    idtr.limit = 256 * 8 - 1;
    idtr.base = (uint32_t)idt;
    asm volatile ("mov eax, %0" :: "r"(&idtr));
    asm volatile ("lidt [eax]");

    fill_idt_entry(0x20, isr_pit_timer);
    fill_idt_entry(0x21, isr_keyboard);

    pic_remap();
    outb(0x21,
            IRQ_MASK_TIMER & IRQ_MASK_KEYBOARD);
    outb(0xa1, 0xff);
    asm ("sti");
}
