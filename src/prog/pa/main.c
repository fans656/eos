#include "stdio.h"
#include "util.h"

int main() {
    for (int i = 0;; ++i) {
        printf("A\n");
        sleep(1000);
    }
}
