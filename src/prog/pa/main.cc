#include "stdio.h"
#include "graphics.h"

int main() {
    char* bmp = (char*)load_file("/img/girl.bmp");
    memory_blit(bmp_data(bmp), bmp_pitch(bmp),
            0, 0, 0, 0,
            bmp_width(bmp), bmp_height(bmp));
}
