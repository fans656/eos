#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"

int main() {
    for (int i = 1; i <= 3; ++i) {
        printf(" B %d\n", i);
        sleep(200);
    }
    printf(" B bye-bye\n");
}
