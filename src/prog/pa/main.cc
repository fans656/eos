#include "stdio.h"
#include "unistd.h"
#include "graphics.h"
#include "list.h"
#include "eos.h"

int main() {
    while (true) {
        int* x = (int*)get_message(0, true);
        printf("%d\n", *x);
        delete x;
    }
}
