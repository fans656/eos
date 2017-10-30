#ifndef LIBC_DEF_H
#define LIBC_DEF_H

#define align4(x) (((x) + 3) / 4 * 4)

enum {
    SYSCALL_EXIT,

    SYSCALL_PRINTF,
    SYSCALL_DEBUG,

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
    SYSCALL_EXECUTE,
    SYSCALL_YIELD,
    SYSCALL_SET_TIMER,
    SYSCALL_TIMEIT,
    SYSCALL_CLOCK,

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

struct ScreenInfo {
    int width;
    int height;
    int bpp;
    int pitch;
    uchar* video_mem;
    uchar* font_data;
    int font_pitch;
    int font_glyph_width;
    int font_glyph_height;
};

enum {
    KEY_EVENT,
    MOUSE_EVENT,
    TIMER_EVENT,
};

struct MouseEvent {
    int type;
    int x;
    int y;
    uint buttons;
    
    MouseEvent(int x, int y, uint buttons)
        : type(MOUSE_EVENT), x(x), y(y), buttons(buttons) {}
};

struct TimerEvent {
    int type;
    uint id;
    
    TimerEvent(uint id) : type(TIMER_EVENT), id(id) {}
};

struct KeyEvent {
    int type;
    uint key;
    bool up;
    
    KeyEvent(uint key, bool up)
        : type(KEY_EVENT), key(key), up(up) {}
};

#endif
