from conf import *
from util import *


class Bitmap(object):

    def __init__(self, disk, meta, new=False):
        self.disk = disk
        self.meta = meta
        if new:
            self.new()
        else:
            self.load()

    def new(self):
        meta = self.meta
        self.a = [0x00] * ((meta.n_blocks + 7) // 8)
        bpb = meta.bytes_per_block
        self.size_in_bytes = size_in_bytes = len(self.a)
        self.size_in_blocks = size_in_blocks = (size_in_bytes + bpb - 1) // bpb
        self.i_block = meta.i_bitmap
        for i in xrange(0, meta.i_free, 8):
            self.alloc(8, i)
        self.flush()

    def load(self):
        meta = self.meta
        bpb = meta.bytes_per_block
        data = self.disk.read_bytes(meta.i_bitmap * bpb, meta.n_blocks // 8)
        self.i_block = meta.i_bitmap
        self.a = [ord(val) for val in data]
        self.size_in_bytes = size_in_bytes = len(self.a)
        self.size_in_blocks = (size_in_bytes + bpb - 1) // bpb

    def alloc(self, n_blocks=1, desired_i_block=None):
        if n_blocks > 8:
            raise Exception('blocks larger than 32K is not allowed')
        valid = False
        if desired_i_block is not None:
            i_block = desired_i_block
            self.validate(i_block, n_blocks)
            if self.available(i_block, n_blocks):
                valid = True
        else:
            try:
                i_block = self.find_free_stride(n_blocks)
                valid = True
            except Exception as e:
                print e
                valid = False
        if valid:
            for i in xrange(i_block, i_block + n_blocks):
                self[i] = True
            return i_block
        raise Exception('can not alloc')

    def find_free_stride(self, n):
        a = self.a
        beg = self.meta.i_free
        if n == 8:
            try:
                return next(i for i in xrange(beg // 8, len(self.a)) if a[i] == 0) * 8
            except StopIteration:
                raise Exception()
        elif n < 8:
            try:
                i = next(i for i in xrange(beg // 8, len(self.a)) if 8 - bit_count(a[i]) >= n)
                mask = (0xff >> (8 - n)) << (8 - n)
                for i_bit in xrange(8):
                    if a[i_bit] & mask == 0:
                        break
                    mask >>= 1
                return i * 8 + i_bit
            except StopIteration:
                raise Exception()
        else:
            raise Exception()

    def free(self, i_block, n_blocks=1):
        self.validate(i_block, n_blocks)
        for i in xrange(i_block, i_block + n_blocks):
            assert self[i]
            self[i] = False

    def __getitem__(self, i_block):
        i_byte = i_block // 8
        i_bit = i_block % 8
        return self.a[i_byte] & (1 << i_bit) != 0

    def __setitem__(self, i_block, used):
        i_byte = i_block // 8
        i_bit = i_block % 8
        if used:
            self.a[i_byte] |= 1 << i_bit
        else:
            self.a[i_byte] &= ~(1 << i_bit)
        self.flush(i_byte)

    def flush(self, i_byte_in_bitmap=None):
        meta = self.meta
        bpb = meta.bytes_per_block
        if i_byte_in_bitmap is None:
            self.disk.write_bytes(meta.i_bitmap * bpb, self.bytes)
        else:
            i_byte_in_disk = self.i_block * bpb + i_byte_in_bitmap
            self.disk.write_bytes(i_byte_in_disk, chr(self.a[i_byte_in_bitmap]))

    @property
    def bytes(self):
        return ''.join(chr(val) for val in self.a)

    def validate(self, i_block, n_blocks):
        beg_i_byte = i_block // 8
        end_i_byte = (i_block + n_blocks) // 8
        return 0 <= beg_i_byte < end_i_byte <= len(self.a)

    def available(self, i_block, n_blocks):
        a = self.a
        for i in xrange(i_block, i_block + n_blocks):
            i_byte = i // 8
            i_bit = i % 8
            if (self.a[i_byte] & (1 << i_bit)) != 0:
                return False
        return True

    def successive_free_blocks_begin_with(self, i_block, n_blocks):
        a = self.a
        for i in xrange(i_block, i_block + n_blocks):
            i_byte = i // 8
            if a[i_byte] == 0xff:
                return 0
            i_bit = i % 8
            if (a[i_byte] & (1 << i_bit)) != 0:
                return i - i_block
        return n_blocks

    def show(self, n_bytes=64):
        meta = self.meta
        i_byte_bitmap = meta.i_bitmap * meta.bytes_per_block
        n_bytes_bitmap = meta.n_blocks // 8
        print '=' * 40, 'bitmap'
        print_hex(self.disk.read_bytes(i_byte_bitmap, n_bytes))
        print '=' * 40, 'bitmap end'


if __name__ == '__main__':
    import os

    from disk import Disk
    from util import *


    class MockMeta(object):

        def __init__(self):
            self.bytes_per_block = BYTES_PER_BLOCK
            self.i_bitmap = I_BITMAP_BLOCK
            self.n_blocks = 16 * MB // BYTES_PER_BLOCK
            n_blocks_bitmap = (
                self.n_blocks // 8 + self.bytes_per_block + 1) // self.bytes_per_block
            self.i_free = self.i_bitmap + n_blocks_bitmap + 1


    if os.path.exists(IMAGE_FPATH):
        os.system('rm {}'.format(IMAGE_FPATH))
    disk = Disk()
    meta = MockMeta()

    i_byte_bitmap = meta.i_bitmap * meta.bytes_per_block
    n_bytes_bitmap = meta.n_blocks // 8

    # test new
    data = '\x01' * n_bytes_bitmap
    disk.write_bytes(i_byte_bitmap, data)
    b = Bitmap(disk, new=True, meta=meta)
    b.flush()
    expected = '\xff' * 32 + '\x07' + '\0' * (512 - 32 - 1)
    assert disk.read_bytes(i_byte_bitmap, n_bytes_bitmap) == expected

    # test load
    b = Bitmap(disk, meta=meta)
    assert ''.join(map(chr, b.a)) == expected



    b = Bitmap(disk, meta)
    b.alloc(1, 259)
    print_hex(disk.read_bytes(i_byte_bitmap, n_bytes_bitmap))

    #print
    #i = b.alloc(6)
    #print_hex(disk.read_bytes(i_byte_bitmap, n_bytes_bitmap))

    #b.free(i, 6)
    #print
    #print_hex(disk.read_bytes(i_byte_bitmap, n_bytes_bitmap))
