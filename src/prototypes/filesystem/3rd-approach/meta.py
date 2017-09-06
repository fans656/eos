import struct

from conf import *


class Meta(object):

    def __init__(self, disk, new=False):
        self.disk = disk
        if new:
            self.bytes_per_block = BYTES_PER_BLOCK
            self.n_blocks = disk.n_bytes // BYTES_PER_BLOCK
            self.i_meta = I_META_BLOCK
            self.i_bitmap = I_BITMAP_BLOCK
            n_blocks_bitmap = (
                self.n_blocks // 8 + self.bytes_per_block + 1
            ) // self.bytes_per_block
            self.i_root = self.i_bitmap + n_blocks_bitmap
            self.i_free = self.i_root + 1
            self.flush()
        else:
            self.load()

    def load(self):
        data = self.disk.read_bytes(I_META_BLOCK * BYTES_PER_BLOCK, BYTES_PER_SECTOR)
        i = 0
        self.n_blocks = struct.unpack('<I', data[i:i+4])[0]
        i += 4
        self.bytes_per_block = struct.unpack('<I', data[i:i+4])[0]
        i += 4
        self.i_meta = struct.unpack('<I', data[i:i+4])[0]
        i += 4
        self.i_bitmap = struct.unpack('<I', data[i:i+4])[0]
        i += 4
        self.i_root = struct.unpack('<I', data[i:i+4])[0]
        i += 4
        self.i_free = struct.unpack('<I', data[i:i+4])[0]
        i += 4

    def flush(self):
        self.disk.write_bytes(I_META_BLOCK * BYTES_PER_BLOCK, self.bytes)

    @property
    def bytes(self):
        s = ''
        s += struct.pack('<I', self.n_blocks)
        s += struct.pack('<I', self.bytes_per_block)
        s += struct.pack('<I', self.i_meta)
        s += struct.pack('<I', self.i_bitmap)
        s += struct.pack('<I', self.i_root)
        s += struct.pack('<I', self.i_free)
        return s

    def show(self):
        print 'n_blocks', self.n_blocks
        print 'bytes_per_block', self.bytes_per_block
        print 'i_meta', self.i_meta
        print 'i_bitmap', self.i_bitmap
        print 'i_root', self.i_root
        print 'i_free', self.i_free


if __name__ == '__main__':
    from disk import Disk
    from util import *


    if os.path.exists(IMAGE_FPATH):
        os.system('rm {}'.format(IMAGE_FPATH))
    disk = Disk()

    # load
    m = Meta(disk)
    m.show()
    print_hex(disk.read_bytes(I_META_BLOCK * BYTES_PER_BLOCK, 32))

    print

    # new
    m = Meta(disk, new=True)
    print_hex(disk.read_bytes(I_META_BLOCK * BYTES_PER_BLOCK, 32))
