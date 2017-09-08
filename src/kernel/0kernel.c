#include "io.h"
#include "interrupt.h"
#include "snake.h"
#include "util.h"
#include "draw.h"
#include "image.h"
#include "disk.h"

void main();

void kernel_entry() {
    clear_screen();
    setup_idt();
    main();
    hlt();
}

void main() {
    init_disk();

    uint8_t buffer[512];
    uint8_t data[512];
    for (int i = 0; i < 512; ++i) {
        data[i] = 0x56;
    }

    read_sector(0, buffer);
    printf("Before write:\n");
    print_mem(buffer, 16);

    write_sector(0, data);

    printf("\nAfter write:\n");
    read_sector(0, buffer);
    print_mem(buffer, 16);
}
