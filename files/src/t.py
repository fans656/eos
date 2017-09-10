# https://en.wikipedia.org/wiki/BMP_file_format

import struct

from f6 import *


class BitmapHeader(object):

    def __init__(self, data):
        self.signature = data[0:2]
        self.file_size = struct.unpack('<I', data[2:6])[0]
        self.offset = struct.unpack('<I', data[10:14])[0]


class BitmapInfoHeader(object):

    def __init__(self, data):
        self.header_size = struct.unpack('<I', data[0:4])[0]
        self.width = struct.unpack('<I', data[4:8])[0]
        self.height = struct.unpack('<I', data[8:12])[0]
        self.planes = struct.unpack('<H', data[12:14])[0]
        self.bpp = struct.unpack('<H', data[14:16])[0]


with open('snow-leopard.bmp', 'rb') as f:
    bh = BitmapHeader(f.read(14))
    print 'Signature:', bh.signature
    print 'File size:', bh.file_size, human_size(bh.file_size)
    print 'Offset:', bh.offset
    print

    bih = BitmapInfoHeader(f.read(40))
    print 'BitmapInfoHeader size:', bih.header_size
    print 'Dimension:', bih.width, 'x', bih.height
    print 'Planes:', bih.planes
    print 'BPP:', bih.bpp
