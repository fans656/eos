#include "zlib.h"
#include "def.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "stdio.h"

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
    int len;
    uint code;
    Leaf(int len = 0) : len(len), code(0) {}
};

struct Symbol {
    int val;
    int len;
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
    HuffmanTree(uint* lens = 0, int n = 0) {
        Leaf* leaves;
        if (lens) {
            leaves = new Leaf[n];
            memset(leaves, 0, max(n, 19) * sizeof(Leaf));
            for (int i = 0; i < n; ++i) leaves[i] = Leaf(lens[i]);
        } else {
            n = 288;
            leaves = new Leaf[n];
            int i = 0;
            for (; i < 144; ++i) leaves[i] = Leaf(8);
            for (; i < 256; ++i) leaves[i] = Leaf(9);
            for (; i < 280; ++i) leaves[i] = Leaf(7);
            for (; i < 288; ++i) leaves[i] = Leaf(8);
        }

        int min = 999, max = 0;
        for (int i = 0; i < n; ++i) {
            int len = leaves[i].len;
            if (len) {
                if (len < min) min = len;
                else if (len > max) max = len;
            }
        }
        max_len = max;

        auto cnt = new int[max + 1];
        memset(cnt, 0, (max + 1) * sizeof(int));
        for (int i = 0; i < n; ++i) {
            if (leaves[i].len) {
                ++cnt[leaves[i].len];
            }
        }
        
        auto next = new int[max + 1];
        memset(next, 0, (max + 1) * sizeof(int));
        for (int i = min; i <= max; ++i) {
            next[i] = (next[i - 1] + cnt[i - 1]) << 1;
        }
        
        for (int i = 0; i < n; ++i) {
            auto& leaf = leaves[i];
            if (leaf.len) {
                leaf.code = next[leaf.len]++;
            }
        }
        
        int nd = 1 << max;
        d = new Symbol[nd];
        memset(d, 0, nd * sizeof(Symbol));
        for (int i = 0; i < n; ++i) {
            auto& leaf = leaves[i];
            auto code = leaf.code;
            auto len = leaf.len;
            if (len) {
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
        
        delete[] leaves;
        delete[] cnt;
        delete[] next;
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

    Symbol* d;
    int max_len;
};

uint parse_length(BitBuffer& buf, uint symbol) {
    if (symbol < 265) {
        return symbol - 254;
    } else if (symbol == 285) {
        return 258;
    } else {
        symbol -= 265;
        auto bits = symbol / 4 + 1;
        auto base = (1 << (bits + 2)) + 3;
        return base + (1 << bits) * (symbol % 4) + buf.pop(bits);
    }
}

uint parse_distance(BitBuffer& buf, HuffmanTree* tree) {
    uint symbol = tree ? tree->next_symbol(buf) : reversed_bits(buf.pop(5), 5);
    if (symbol < 4) {
        return symbol + 1;
    } else if (symbol < 30) {
        auto bits = (symbol >> 1) - 1;
        auto base = (1 << (bits + 1)) + 1;
        auto diff = (1 << bits) * (symbol - (symbol >> 1 << 1));
        return base + diff + buf.pop(bits);
    }
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
