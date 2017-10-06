#ifndef UTIL_H
#define UTIL_H

static inline void sleep(int ms) {
    for (int i = 0; i < 100000 * ms; ++i) {
        ;
    }
}

#endif
