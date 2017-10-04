#include "stdio.h"
#include "unistd.h"

int main() {
    for (int i = 1; i <= 3; ++i) {
        printf("A %d\n", i);
        sleep(500);
    }
    printf("A bye-bye\n");
}
