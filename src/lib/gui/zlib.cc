#include "zlib.h"
#include "def.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "stdio.h"
#include "time.h"
#include "unistd.h"

class BitBuffer {
public:
    BitBuffer(const uchar* data) : data((uchar*)data), i_byte(0), bits(0), n_bits(0) {}

    uint pop(int n) {
        uint res = peek(n);
        remove(n);
        return res;
    }

    uint peek(int n) {
        while (n_bits < n) {
            if (n_bits) {
                bits |= data[i_byte++] << n_bits;
            } else {
                bits |= data[i_byte++];
            }
            n_bits += 8;
        }
        return bits & ((1 << n) - 1);
    }
    
    void remove(int n) {
        bits >>= n;
        n_bits -= n;
    }
    
    void align(int n) {
        int m = n_bits % n;
        if (m) {
            n_bits -= m;
            bits >>= m;
        }
    }
    
    const uchar* data;
    int i_byte;
    uint bits;
    uint n_bits;
};

struct Leaf {
    uchar len;
    ushort code;
    Leaf(int len = 0) : len(len), code(0) {}
};

struct Symbol {
    int val;
    uchar len;
    Symbol(int val = 0, int len = 0) : val(val), len(len) {}
};

uint reversed_bits(uint x, int n) {
    uint y = 0;
    while (n--) {
        y = (y << 1) | (x & 1);
        x >>= 1;
    }
    return y;
}

class HuffmanTree {
public:
    HuffmanTree(uint* lens = DEF_LENS, int n = 288) {
        if (lens == DEF_LENS && lens[0] == 0) {
            int i = 0;
            for (; i < 144; ++i) lens[i] = 8;
            for (; i < 256; ++i) lens[i] = 9;
            for (; i < 280; ++i) lens[i] = 7;
            for (; i < 288; ++i) lens[i] = 8;
        }

        int min = MAX_CODE_LEN, max = 0;
        for (int i = 0; i < n; ++i) {
            auto len = lens[i];
            if (len) {
                if (len < min) min = len;
                else if (len > max) max = len;
            }
        }
        max_len = max;

        int cnt[MAX_CODE_LEN] = {0};
        for (int i = 0; i < n; ++i) {
            auto len = lens[i];
            if (len) {
                ++cnt[len];
            }
        }
        
        int next[MAX_CODE_LEN] = {0};
        for (int i = min; i <= max; ++i) {
            next[i] = (next[i - 1] + cnt[i - 1]) << 1;
        }
        
        d = new Symbol[1 << max];
        for (int i = 0; i < n; ++i) {
            auto len = lens[i];
            if (len) {
                auto code = next[len]++;
                if (len == max) {
                    d[reversed_bits(code, len)] = Symbol(i, len);
                } else {
                    int m = 1 << (max - len);
                    for (int x = 0; x < m; ++x) {
                        d[(x << len) | reversed_bits(code, len)] = Symbol(i, len);
                    }
                }
            }
        }
    }

    ~HuffmanTree() {
        delete[] d;
    }
    
    uint next_symbol(BitBuffer& buf) {
        auto code = buf.peek(max_len);
        auto symbol = d[code];
        buf.remove(symbol.len);
        return symbol.val;
    }
    
    static constexpr int MAX_CODE_LEN = 16;
    static uint DEF_LENS[288];

    Symbol* d;
    int max_len;
};

uint HuffmanTree::DEF_LENS[288] = {0};

uint parse_length(BitBuffer& buf, uint symbol) {
    static constexpr ushort SYM_TO_LEN[] = {
        /* 257 */   3, /* 258 */   4, /* 259 */   5,
        /* 260 */   6, /* 261 */   7, /* 262 */   8, /* 263 */   9, /* 264 */  10,
        /* 265 */  11, /* 266 */  13, /* 267 */  15, /* 268 */  17, /* 269 */  19,
        /* 270 */  23, /* 271 */  27, /* 272 */  31, /* 273 */  35, /* 274 */  43,
        /* 275 */  51, /* 276 */  59, /* 277 */  67, /* 278 */  83, /* 279 */  99,
        /* 280 */ 115, /* 281 */ 131, /* 282 */ 163, /* 283 */ 195, /* 284 */ 227,
        /* 285 */ 258
    };
    static constexpr uchar EXTRA_BITS[] = {
        /* 257 */ 0, /* 258 */ 0, /* 259 */ 0, /* 260 */ 0,
        /* 261 */ 0, /* 262 */ 0, /* 263 */ 0, /* 264 */ 0,
        /* 265 */ 1, /* 266 */ 1, /* 267 */ 1, /* 268 */ 1,
        /* 269 */ 2, /* 270 */ 2, /* 271 */ 2, /* 272 */ 2,
        /* 273 */ 3, /* 274 */ 3, /* 275 */ 3, /* 276 */ 3,
        /* 277 */ 4, /* 278 */ 4, /* 279 */ 4, /* 280 */ 4,
        /* 281 */ 5, /* 282 */ 5, /* 283 */ 5, /* 284 */ 5,
        /* 285 */ 0
    };
    symbol -= 257;
    return SYM_TO_LEN[symbol] + buf.pop(EXTRA_BITS[symbol]);
}

uint parse_distance(BitBuffer& buf, HuffmanTree* tree) {
    static constexpr ushort SYM_TO_DIS[] = {
         1,     2,    3,    4,    5,    7,    9,    13,    17,    25,
        33,    49,   65,   97,  129,  193,  257,   385,   513,   769,
      1025,  1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577,
    };
    static constexpr uchar EXTRA_BITS[] = {
        0,  0,  1,  2,  3, 4,  5,  6,  7,  8, 9, 10, 11, 12, 13, 14
    };
    uint symbol = tree ? tree->next_symbol(buf) : reversed_bits(buf.pop(5), 5);
    return SYM_TO_DIS[symbol] + buf.pop(EXTRA_BITS[symbol / 2]);
}

uchar* inflate(BitBuffer& buf, uchar* dst, HuffmanTree* lit_tree, HuffmanTree* dis_tree = 0) {
    for (int i = 0;; ++i) {
        auto symbol = lit_tree->next_symbol(buf);
        if (symbol < 256) {
            *dst++ = symbol;
        } else if (symbol == 256) {
            break;
        } else if (symbol < 288) {
            auto len = parse_length(buf, symbol);
            auto dis = parse_distance(buf, dis_tree);
            memcpy(dst, dst - dis, len);
            dst += len;
        }
    }
    return dst;
}

static int ORDER[] = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};

void parse_tree(BitBuffer& buf, HuffmanTree*& lit_tree, HuffmanTree*& dis_tree) {
    auto hlit = buf.pop(5) + 257;
    auto hdis = buf.pop(5) + 1;
    auto hlen = buf.pop(4) + 4;
    uint lens[19] = {0};
    for (int i = 0; i < hlen; ++i) {
        auto val = buf.pop(3);
        lens[ORDER[i]] = val;
    }
    HuffmanTree len_tree(lens, 19);
    
    auto all_lens = new uint[hlit + hdis];
    auto last_symbol = 0;
    for (int i = 0; i < hlit + hdis;) {
        auto symbol = len_tree.next_symbol(buf);
        auto repeat = 1;
        switch (symbol) {
            case 16:
                repeat = 3 + buf.pop(2);
                break;
            case 17:
                repeat = 3 + buf.pop(3);
                last_symbol = 0;
                break;
            case 18:
                repeat = 11 + buf.pop(7);
                last_symbol = 0;
                break;
            default:
                repeat = 1;
                last_symbol = symbol;
        }
        while (repeat--) all_lens[i++] = last_symbol;
    }
    lit_tree = new HuffmanTree(all_lens, hlit);
    dis_tree = new HuffmanTree(all_lens + hlit, hdis);
}

void decompress(uchar* dst, const uchar* src) {
    BitBuffer buf(src);
    auto CMF = buf.pop(8);
    auto FLG = buf.pop(8);
    HuffmanTree def_tree;
    bool bfinal = false;
    while (!bfinal) {
        bfinal = buf.pop(1);
        auto btype = buf.pop(2);
        switch (btype) {
            case 0:
                {
                    buf.align(8);
                    auto len = buf.pop(16);
                    auto clen = buf.pop(16);
                    while (len--) {
                        *dst++ = buf.pop(8);
                    }
                }
                break;
            case 1:
                dst = inflate(buf, dst, &def_tree, 0);
                break;
            case 2:
                {
                    HuffmanTree* lit_tree;
                    HuffmanTree* dis_tree;
                    parse_tree(buf, lit_tree, dis_tree);
                    dst = inflate(buf, dst, lit_tree, dis_tree);
                }
                break;
        }
    }
}
