#include "stdlib.h"

int cur_rand_val;

int rand() {
    cur_rand_val = (22695477 * (uint)cur_rand_val + 1) % RAND_MAX;
    return cur_rand_val;
}

void srand(uint seed) {
    cur_rand_val = seed;
    rand();
}

int randint(int min, int max) {
    uint range = max - min + 1;
    if (range <= RAND_MAX) {
       return min + rand() % range;
    } else {
        int n = range / RAND_MAX;
        int i = randint(0, n);
        return RAND_MAX * i + randint(0, (i == n) ? (range % RAND_MAX) : (RAND_MAX - 1));
    }
}
