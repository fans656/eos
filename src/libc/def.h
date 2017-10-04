#ifndef LIBC_DEF_H
#define LIBC_DEF_H

enum {
    SYSCALL_EXIT = 0,

    SYSCALL_PRINTF,

    SYSCALL_MALLOC,
    SYSCALL_FREE,

    SYSCALL_FOPEN,
    SYSCALL_FCLOSE,
    SYSCALL_FREAD,
    SYSCALL_FWRITE,
    SYSCALL_FSIZE,
    SYSCALL_LOAD_FILE,

    SYSCALL_BMP_BLIT,
};

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long long ulonglong;
typedef uint size_t;

#define bool uchar
#define true 1
#define false 0

#endif
