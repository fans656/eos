#include "types.h"
#include "stdio.h"

extern uint* kernel_end;

void main() {
    printf("%x", *kernel_end);
}
