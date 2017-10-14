#include "stdio.h"
#include "unistd.h"

int main() {
    for (int i = 0; ; ++i) {
        sleep(1000);
        printf("Proc B %d\n", i);
    }
}
