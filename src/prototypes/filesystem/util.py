from conf import *


def print_hex(s, limit=float('inf')):
    for i in xrange(0, min(len(s), limit), 32):
        part = s[i:i+32]
        print ' '.join('{:02x}'.format(ord(t)) for t in part)


def bytes_to_block(n):
    return (n + BYTES_PER_BLOCK - 1) // BYTES_PER_BLOCK
