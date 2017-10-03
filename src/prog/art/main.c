#include "stdio.h"
#include "stdlib.h"
#include "graphics.h"

int main() {
    printf("in prog\n");
    void* girl = load_file("/img/girl.bmp");
    void* walle = load_file("/img/walle.bmp");

    draw_bmp_at(girl, 100, 100);
    draw_bmp_at(walle, 500, 200);
    
    free(girl);
    free(walle);
}
