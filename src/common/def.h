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

    SYSCALL_SLEEP,
    SYSCALL_SET_TIMER,

    SYSCALL_MEMORY_BLIT,

    SYSCALL_BMP_BLIT,
    
    SYSCALL_GET_MESSAGE,
    SYSCALL_PUT_MESSAGE,

    SYSCALL_INIT_GUI,
};

enum {
    GUI_MESSAGE_ID,
    GUI_KEYBOARD_EVENT_ID,
    GUI_MOUSE_EVENT_ID,
};

enum {
    MESSAGE_TIMER = 1,
};

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long long ulonglong;
typedef uint size_t;

#define bool uchar
#define true 1
#define false 0

struct GUIInfo {
    int screen_width;
    int screen_height;
    int screen_pitch;
    int screen_bpp;
    
    GUIInfo(int width, int height, int pitch, int bpp)
        : screen_width(width), screen_height(height),
        screen_pitch(pitch), screen_bpp(bpp) {
    }
};

struct GUIMouseEvent {
    ushort x, y;
    bool left;
    bool right;
};

struct KernelTimerEvent {
    uint type;
    uint id;
    KernelTimerEvent(uint id) : type(MESSAGE_TIMER), id(id) {}
};

#endif
