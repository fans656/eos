import os

from conf import *
from util import *


class Disk(object):

    def __init__(self,
                 n_bytes=DISK_SIZE_IN_BYTES,
                 bytes_per_sector=BYTES_PER_SECTOR,
                 image_fpath=IMAGE_FPATH):
        if n_bytes % bytes_per_sector != 0:
            raise Exception('{} is not a multiple of {}'.format(
                n_bytes, bytes_per_sector))

        self.n_bytes = n_bytes
        self.bytes_per_sector = bytes_per_sector
        self.n_sectors = n_bytes // bytes_per_sector
        self.image_fpath = image_fpath

        self.open_image_file(image_fpath)

    def open_image_file(self, fpath):
        self.new = False
        if self.not_exists(fpath) or self.wrong_size(fpath):
            create_image_file(fpath, self.n_bytes)
            self.new = True
        self.f = open(fpath, 'rb+')

    def not_exists(self, fpath):
        return not os.path.exists(fpath)

    def wrong_size(self, fpath):
        return os.stat(fpath).st_size != self.n_bytes

    def close(self):
        self.f.close()

    def read_bytes(self, i_byte, n_bytes):
        byte_beg, byte_end, sector_beg, sector_end = self.byte_sector_range(
            i_byte, n_bytes)
        data = ''
        for i_sector in xrange(sector_beg, sector_end):
            data += self.read_sector(i_sector)
        base = i_byte % self.bytes_per_sector
        return data[base:base+n_bytes]

    def write_bytes(self, i_byte, data):
        bps = self.bytes_per_sector
        n_bytes = len(data)
        byte_beg, byte_end, sector_beg, sector_end = self.byte_sector_range(
            i_byte, n_bytes)
        n_bytes_remained = n_bytes
        data_offset = 0
        for i_sector in xrange(sector_beg, sector_end):
            n_left = i_byte % bps
            at_sector_begin = n_left != 0
            at_sector_end = n_bytes_remained >= bps
            if not at_sector_begin or not at_sector_end:
                sector_data = self.read_sector(i_sector)
                n_taken = min(bps - n_left, n_bytes_remained)
                sector_data = (
                    sector_data[:n_left]
                    + data[data_offset:data_offset+n_taken]
                    + sector_data[n_left+n_taken:]
                )
            else:
                i = data_offset
                n_taken = bps
                sector_data = data[i:i+n_taken]
            self.write_sector(i_sector, sector_data)
            data_offset += n_taken
            i_byte += n_taken
            n_bytes_remained -= n_taken

    def read_sector(self, i_sector):
        self.validate_sector_offset(i_sector)
        self.f.seek(i_sector * self.bytes_per_sector, os.SEEK_SET)
        return self.f.read(self.bytes_per_sector)

    def write_sector(self, i_sector, data):
        self.validate_data_length(data)
        self.validate_sector_offset(i_sector)
        self.f.seek(i_sector * self.bytes_per_sector, os.SEEK_SET)
        self.f.write(data)

    def validate_data_length(self, data):
        assert len(data) == self.bytes_per_sector

    def validate_sector_offset(self, i_sector):
        assert 0 <= i_sector < self.n_sectors

    def byte_sector_range(self, i_byte, n_bytes):
        bps = self.bytes_per_sector
        byte_beg = i_byte
        byte_end = i_byte + n_bytes
        sector_beg = byte_beg // bps
        sector_end = (byte_end + bps - 1) // bps
        return byte_beg, byte_end, sector_beg, sector_end


if __name__ == '__main__':
    # wrong disk size
    try:
        Disk(1)
        raise Exception()
    except Exception:
        pass

    # without disk image
    d = Disk(16 * MB)
    assert os.path.exists(IMAGE_FPATH)
    os.system('rm {}'.format(IMAGE_FPATH))

    # with disk image of different size
    assert not os.path.exists(IMAGE_FPATH)
    d = Disk(16 * MB)
    assert os.path.exists(IMAGE_FPATH)
    assert os.stat(IMAGE_FPATH).st_size == 16 * MB
    d = Disk(8 * MB)
    assert os.path.exists(IMAGE_FPATH)
    assert os.stat(IMAGE_FPATH).st_size == 8 * MB
    os.system('rm {}'.format(IMAGE_FPATH))

    # with disk image of same size
    assert not os.path.exists(IMAGE_FPATH)
    d = Disk(16 * MB)
    assert os.path.exists(IMAGE_FPATH)
    assert os.stat(IMAGE_FPATH).st_size == 16 * MB
    d = Disk(16 * MB)
    assert os.path.exists(IMAGE_FPATH)
    assert os.stat(IMAGE_FPATH).st_size == 16 * MB
    os.system('rm {}'.format(IMAGE_FPATH))
    assert not os.path.exists(IMAGE_FPATH)


    def read_write_test(i_byte, data):
        d = Disk()
        f = d.f
        n = len(data)
        # read test
        f.seek(i_byte)
        f.write(data)
        assert d.read_bytes(i_byte, n) == data
        # write test
        d.write_bytes(i_byte, data)
        f.seek(i_byte)
        assert f.read(n) == data

    for pos in [
            0, 1, 256, 511,
            512, 513, 513 + 256, 1024 - 1,
    ]:
        for data in [
                'x', 'ab', 'x' * 513, 'x' * 1025, 'x' * (3 * MB + 7)
        ]:
            read_write_test(pos, data)
