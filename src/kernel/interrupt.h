#ifndef INTERRUPT_H
#define INTERRUPT_H

typedef struct _IDTEntry {
    uint16_t offset1;
    uint16_t selector;
    uint8_t zero;
    uint8_t attr;
    uint16_t offset2;
} IDTEntry;

extern struct _IDTEntry idt[256];
void setup_idt();

#endif
