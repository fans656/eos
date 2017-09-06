import os


def create_image_file(fpath, size_in_bytes):
    os.system('dd if=/dev/zero of={} bs={} count=1 >/dev/null 2>&1'.format(
        fpath, size_in_bytes
    ))


def print_hex(s, limit=float('inf')):
    for i in xrange(0, min(len(s), limit), 32):
        part = s[i:i+32]
        print ' '.join('{:02x}'.format(ord(t)) for t in part)
