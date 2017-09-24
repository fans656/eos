#include "types.h"
#include "conf.h"

#define V2P(x) ((uchar*)((uint)(x) - KERNEL_BASE))
#define P2V(x) ((uchar*)((uint)(x) + KERNEL_BASE))

#define V2P_UINT(x) ((uint)(x) - KERNEL_BASE)
#define P2V_UINT(x) ((uint)(x) + KERNEL_BASE)
