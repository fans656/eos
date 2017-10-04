#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "graphics.h"

int main() {
    uint* bmp = load_file("/img/walle.bmp");
    for (int y = 0; y <= 300; y += 5) {
        sleep(1);
        draw_bmp_at(bmp, 500, y);
    }
}
