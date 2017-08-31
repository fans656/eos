#include <stdint.h>
#include "interrupt.h"
#include "io.h"
#include "util.h"

#define IRQ_KEYBOARD 1
#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xa0
#define PIC2_DATA 0xa1

#define PIC_EOI 0x20

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

void send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}

int counter = 0;

void isr_default() {
    asm volatile ("pushad");
    
    print_str("isr_default ");
    print_int(counter++);
    put_char('\n');

    send_eoi(255);
    asm volatile ("popad; leave; iret");
}

void isr_keyboard() {
    asm volatile ("pushad");
    
    print_str("isr_keyboard\n");

    asm volatile ("popad; leave; iret");
}

void setup_idt() {
    IDTR idtr;
    idtr.limit = 256 * 8 - 1;
    idtr.base = (uint32_t)idt;
    asm volatile ("mov eax, %0" :: "r"(&idtr));
    asm volatile ("lidt [eax]");

    for (int i = 0; i < 256; ++i) {
        fill_idt_entry(i, isr_default);
    }
    fill_idt_entry(0x02, isr_keyboard);

    outb(0x21, 0xfe);
    outb(0xa1, 0xff);

    //asm("int 2");
    asm ("sti");
}
