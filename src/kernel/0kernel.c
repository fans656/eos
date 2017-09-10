#include "io.h"
#include "util.h"
#include "draw.h"
#include "interrupt.h"
#include "filesystem.h"
#include "malloc.h"

void main();

void kernel_entry() {
    setup_idt();
    init_memory();
    graphic_init();
    init_filesystem();
    clear_screen();
    main();
    hlt();
}

char* images[] = {
    "/images/snow-leopard.bmp",
    "/images/three-body.bmp",
    "/images/girl.bmp",
};

void main() {
    int n = sizeof(images) / sizeof(images[0]);
    char* image_contents[n];
    for (int i = 0; i < n; ++i) {
        FILE* fp = fopen(images[i]);
        uint64_t size = fp->entry->size;
        image_contents[i] = malloc(size);
        fread(fp, size, image_contents[i]);
    }
    int i = 0;
    while (true) {
        draw_bmp(image_contents[i]);
        sleep(3000);
        screen_fill_black();
        i = (i + 1) % n;
    }
}
