#ifndef UTIL_H
#define UTIL_H

#include "def.h"

#define align4(x) (((x) + 3) / 4 * 4)

#define restricted(x, mi, ma) (min((ma), (max((x), (mi)))))

void hlt_forever();
void hexdump(void* addr, uint cnt);

///////////////////////////////////////////////////////////////////

// https://stackoverflow.com/a/28139075/3881776
template<class C> struct Reversed { C& c; };
template<class C> auto begin(Reversed<C> c) { return c.c.rbegin(); }
template<class C> auto end(Reversed<C> c) { return c.c.rend(); }
template<class C> Reversed<C> reversed(C& c) { return {c}; }

#endif
