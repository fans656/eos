#include "gui.h"
#include "stdio.h"
#include "unistd.h"
#include "eos.h"

int main() {
    Bitmap* imgs[2];
    imgs[0] = new Bitmap("/img/snow-leopard.bmp");
    printf("snow-leopard read\n");
    imgs[1] = new Bitmap("/img/cheetah.png");
    printf("cheetah read\n");
    for (int i = 0; ; ++i) {
        auto img = imgs[i % 2];
        memory_blit(img->data(), img->pitch(),
                0, 0, 0, 0, 800, 600);
        sleep(10);
    }
    //gui_server();
}
