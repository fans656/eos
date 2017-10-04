#include "stdio.h"
#include "util.h"

int main() {
    for (int i = 0; i < 5; ++i) {
        printf(" B %d\n", i);
        sleep(100);
    }
}
