#ifndef STR_H
#define STR_H
    
namespace StrHelper {
    size_t cstr_len(const char* s) {
        size_t n = 0;
        while (*s++) {
            ++n;
        }
        return n;
    }
    
    size_t cstr_copy(char* dst, const char* src) {
        while (*src) {
            *dst++ = *src++;
        }
        *dst = 0;
    }
}

struct Str {
    Str(const char* s) {
        len = StrHelper::cstr_len(s);
        this->s = new char[len + 1];
        StrHelper::cstr_copy(this->s, s);
    }
    const char* cstr() const { return s; }
    
    char* s;
    size_t len;
};

#endif
