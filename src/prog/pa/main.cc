#include "stdio.h"
#include "unistd.h"
#include "graphics.h"
#include "list.h"
#include "eos.h"

int main() {
    for (int i = 0; i < 3; ++i) {
        printf("%d\n", i);
        sleep(500);
    }
}
