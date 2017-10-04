#include "stdio.h"
#include "unistd.h"
#include "graphics.h"

int main() {
    uint* bmp = load_file("/img/girl.bmp");
    draw_bmp_at(bmp, 0, 0);
}
