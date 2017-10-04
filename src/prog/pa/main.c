#include "stdio.h"
#include "util.h"

int main() {
    int r = printf("A hello, fans%d\n", 656);
    printf("A printf return %d\n", r);
    sleep(1000);
    printf("A bye-bye!\n");
    //for (int i = 0;; ++i) {
    //    printf("A\n");
    //    sleep(1000);
    //}
}
