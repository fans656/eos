#ifndef UTIL_H
#define UTIL_H

#include "def.h"

#define align4(x) (((x) + 3) / 4 * 4)

#define restricted(x, mi, ma) (min((ma), (max((x), (mi)))))

void hlt_forever();

void hexdump(void* addr, uint cnt);

#endif
