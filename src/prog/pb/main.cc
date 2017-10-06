#include "stdio.h"
#include "unistd.h"
#include "graphics.h"
#include "list.h"
#include "eos.h"

int main() {
    for (int i = 0; i < 5; ++i) {
        printf(" B-%d\n", i);
        sleep(200);
    }
}
