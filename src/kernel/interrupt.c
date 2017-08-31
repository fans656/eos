#include <stdint.h>
#include "interrupt.h"
#include "io.h"
#include "util.h"

#define IRQ_KEYBOARD 1

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

void isr_keyboard() {
    asm volatile ("pushad");
    
    clear_screen();
    print_str("isr_keyboard\n");

    asm volatile ("popad; leave; iret");
}

void setup_idt() {
    IDTR idtr;
    idtr.limit = 256 * 8 - 1;
    idtr.base = (uint32_t)idt;
    asm volatile ("mov eax, %0" :: "r"(&idtr));
    asm volatile ("lidt [eax]");

    fill_idt_entry(49, isr_keyboard);

    //asm ("sti");
}
