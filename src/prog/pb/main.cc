#include "stdio.h"
#include "unistd.h"
#include "graphics.h"
#include "list.h"
#include "eos.h"

int main() {
    for (int i = 0; i < 10; ++i) {
        put_message(0, new int(i));
    }
}
