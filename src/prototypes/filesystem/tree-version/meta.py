
import struct


class Meta(object):

    def __init__(self, data=None):
        if data:
            i = 0
            self.n_sectors = struct.unpack('<I', data[i:i+4])[0]
            i += 4
            self.n_blocks_bitmap = struct.unpack('<I', data[i:i+4])[0]
            i += 4

    @property
    def bytes(self):
        s = ''
        s += struct.pack('<I', self.n_sectors)
        s += struct.pack('<I', self.n_blocks_bitmap)
        return s

    def show(self):
        print 'Number of sectors', self.n_sectors
        print 'Blocks of bitmap', self.n_blocks_bitmap
