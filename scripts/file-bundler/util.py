import os


def create_image_file(fpath, size_in_bytes):
    os.system('dd if=/dev/zero of={} bs={} count=1 >/dev/null 2>&1'.format(
        fpath, size_in_bytes
    ))


def print_hex(s, limit=float('inf')):
    for i in xrange(0, min(len(s), limit), 32):
        part = s[i:i+32]
        print ' '.join('{:02x}'.format(ord(t)) for t in part)


def bit_count(x):
    x = ((x & 0xaaaaaaaa) >> 1) + (x & 0x55555555)
    x = ((x & 0xcccccccc) >> 2) + (x & 0x33333333)
    x = ((x & 0xf0f0f0f0) >> 4) + (x & 0x0f0f0f0f)
    x = ((x & 0xff00ff00) >> 8) + (x & 0x00ff00ff)
    x = ((x & 0xffff0000) >> 16) + (x & 0x0000ffff)
    return x
