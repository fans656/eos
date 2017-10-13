#include "png.h"
#include "string.h"
#include "zlib.h"
#include "math.h"
#include "stdio.h"
#include "stdlib.h"

#define MAKE_DWORD(a,b,c,d) (((a) << 24) | ((b) << 16) | ((c) << 8) | d)

constexpr uint IHDR = MAKE_DWORD('I', 'H', 'D', 'R');
constexpr uint IDAT = MAKE_DWORD('I', 'D', 'A', 'T');
constexpr uint IEND = MAKE_DWORD('I', 'E', 'N', 'D');

class Buffer {
public:
    Buffer(const char* data) : data(data), i_byte(0) {
    }
    
private:
    const char* data;
    int i_byte;
};

uint hton(uint x) {
    return ((x & 0xff) << 24)
        | (((x >> 8) & 0xff) << 16)
        | (((x >> 16) & 0xff) << 8)
        | ((x >> 24) & 0xff);
}

inline char paeth(char a, char b, char c) {
    auto p = a + b - c;
    auto pa = abs(p - a);
    auto pb = abs(p - b);
    auto pc = abs(p - c);
    if (pa <= pb && pa <= pc) {
        return a;
    } else if (pb <= pc) {
        return b;
    } else {
        return c;
    }
}

void unfilter(int type, uchar* dst, uchar* line, uchar* pline, int n) {
    switch (type) {
        case 0:
            while (n--) {
                *dst++ = *line++;
            }
            break;
        case 1:
            for (int i = 0; i < 4; ++i) {
                *dst++ = *line++;
            }
            for (int i = 4; i < n; ++i) {
                *dst++ = *line = *line + *(line - 4);
                ++line;
            }
            break;
        case 2:
            if (pline) {
                for (int i = 0; i < n; ++i) {
                    *dst++ = *line = *line + *pline++;
                    ++line;
                }
            } else {
                for (int i = 0; i < n; ++i) {
                    *dst++ = *line++;
                }
            }
            break;
        case 3:
            if (pline) {
                for (int i = 0; i < 4; ++i) {
                    *dst++ = *line = *line + *pline++ / 2;
                    ++line;
                }
                for (int i = 4; i < n; ++i) {
                    *dst++ = *line = *line + (*(line - 4) + *pline++) / 2;
                    ++line;
                }
            } else {
                for (int i = 0; i < 4; ++i) {
                    *dst++ = *line++;
                }
                for (int i = 4; i < n; ++i) {
                    *dst++ = *line = *line + *(line - 4) / 2;
                    ++line;
                }
            }
            break;
        case 4:
            exit(0);
            if (pline) {
                for (int i = 0; i < 4; ++i) {
                    *dst++ = *line = *line + paeth(0, *pline++, 0);
                    ++line;
                }
                for (int i = 4; i < n; ++i) {
                    *dst++ = *line = *line + paeth(*(line - 4), *pline, *(pline - 4));
                    ++line;
                    ++pline;
                }
            } else {
                for (int i = 0; i < 4; ++i) {
                    *dst++ = *line++;
                }
                for (int i = 4; i < n; ++i) {
                    *dst++ = *line = *line + *(line - 4);
                    ++line;
                }
            }
            break;
    }
}

uchar* load_png(const char* path, int& width, int& height) {
    uchar* data = (uchar*)load_file(path);
    data += 8;  // skip signature
    uchar* idat;
    size_t idat_len;
    int line_pitch;
    uchar* p = data;
    while (true) {
        uint chunk_len = hton(*(uint*)p); p += 4;
        uint chunk_type = hton(*(uint*)p); p += 4;
        if (chunk_type == IEND) {
            break;
        } else if (chunk_type == IHDR) {
            width = hton(*(uint*)(p));
            height = hton(*(uint*)(p + 4));
            line_pitch = width * 4 + 1;
            p += chunk_len;
        } else if (chunk_type == IDAT) {
            idat_len += chunk_len;
            p += chunk_len;
        } else {
            p += chunk_len;
        }
        p += 4;
    }
    p = data;
    idat = new uchar[idat_len];
    uchar* q = idat;
    while (true) {
        uint chunk_len = hton(*(uint*)p); p += 4;
        uint chunk_type = hton(*(uint*)p); p += 4;
        if (chunk_type == IDAT) {
            memcpy(q, p, chunk_len);
            q += chunk_len;
            p += chunk_len;
        } else if (chunk_type == IEND) {
            break;
        } else {
            p += chunk_len;
        }
        p += 4;
    }
    uchar* raw = new uchar[line_pitch * height];
    decompress(raw, idat);
    delete[] idat;
    uchar* out = new uchar[width * height * 4];
    for (int y = 0; y < height; ++y) {
        uchar* line = raw + 1 + y * line_pitch;
        uchar* pline = y > 0 ? line - line_pitch : 0;
        uchar* oline = out + y * width * 4;
        int filter_type = *(line - 1);
        unfilter(filter_type, oline, line, pline, width * 4);
        for (int x = 0; x < width; ++x) {
            uint& color = *(uint*)(out + (y * width + x) * 4);
            // PNG color is RGBA (byte order from lower address to higher address)
            // VESA color is BGRA (byte order from lower address to higher address)
            uchar a = (color >> 24) & 0xff;
            uchar b = (color >> 16) & 0xff;
            uchar g = (color >> 8) & 0xff;
            uchar r = color & 0xff;
            color = (a << 24) | (r << 16) | (g << 8) | b;
        }
    }
    delete[] raw;
    return out;
}
