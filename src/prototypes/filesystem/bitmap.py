import struct

from conf import *
from util import *


class Bitmap(object):

    def __init__(self, n_sectors_or_data):
        if isinstance(n_sectors_or_data, int):
            n_sectors_disk = n_sectors_or_data
            n_blocks_disk = n_sectors_disk // SECTORS_PER_BLOCK
            self.a = [0] * ((n_blocks_disk + 7) // 8)
        else:
            data = n_sectors_or_data
            n_bytes_data = len(data)
            n_blocks_disk = len(data) * 8
            self.a = [struct.unpack('B', val)[0] for val in data]

    def alloc_block(self):
        for i, val in enumerate(self.a):
            if val != 0xff:
                break
        i_block = i * 8
        for i in xrange(8):
            if val & (1 << i) == 0:
                i_block += i
                break
        self.mark_block(i_block, True)
        return i_block

    def __setitem__(self, idx, val):
        if isinstance(idx, slice):
            for i in xrange(idx.start, idx.stop):
                self.mark_block(i, val)
        else:
            self.mark_block(idx, val)

    def mark_block(self, i, val):
        i_byte = i // 8
        i_bit = i % 8
        if val:
            self.a[i_byte] |= 1 << i_bit
        else:
            self.a[i_byte] &= ~(1 << i_bit)

    def show(self, limit=None):
        print_hex(self.bytes, limit=limit)

    @property
    def bytes(self):
        return ''.join(struct.pack('B', val) for val in self.a)

    def __len__(self):
        return len(self.a)
