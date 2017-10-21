#ifndef LIBC_DEF_H
#define LIBC_DEF_H

#define align4(x) (((x) + 3) / 4 * 4)

enum {
    SYSCALL_EXIT,

    SYSCALL_PRINTF,

    SYSCALL_MALLOC,
    SYSCALL_FREE,

    SYSCALL_FOPEN,
    SYSCALL_FCLOSE,
    SYSCALL_FREAD,
    SYSCALL_FWRITE,
    SYSCALL_FSIZE,
    SYSCALL_LOAD_FILE,

    SYSCALL_IS_IDLE,
    SYSCALL_SLEEP,
    SYSCALL_YIELD,
    SYSCALL_SET_TIMER,
    SYSCALL_TIMEIT,

    SYSCALL_BLIT,
    
    SYSCALL_GET_MESSAGE,
    SYSCALL_PUT_MESSAGE,
    SYSCALL_REPLACE_MESSAGE,
    
    SYSCALL_GET_SCREEN_INFO,
};

enum {
    QUEUE_ID_GUI,
};

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long long ulonglong;
typedef uint size_t;

#define bool uchar
#define true 1
#define false 0

struct ScreenInfo {
    int width;
    int height;
    int bpp;
    int pitch;
    uchar* video_mem;
};

enum {
    MOUSE_EVENT,
};

struct MouseEvent {
    int type;
    int x;
    int y;
    uint buttons;
    
    MouseEvent(int x, int y, uint buttons)
        : type(MOUSE_EVENT), x(x), y(y), buttons(buttons) {}
};

#endif
