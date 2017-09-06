import os

from conf import *


class Hardware(object):

    def __init__(self, image_fpath, new):
        self.is_new = False
        if not os.path.exists(image_fpath) or new:
            print 'Creating default disk of 16M'
            os.system('dd if=/dev/zero of={} bs=16M count=1 && clear'.format(image_fpath))
            self.is_new = True
        self.image_fpath = image_fpath
        self.f = open(image_fpath, 'rb+')

    @property
    def n_sectors(self):
        return len(self) // BYTES_PER_SECTOR

    def __len__(self):
        saved_pos = self.f.tell()
        self.f.seek(0, os.SEEK_END)
        r = self.f.tell()
        self.f.seek(saved_pos, os.SEEK_SET)
        return r

    def read(self, i_sector, n_sectors):
        self.f.seek(i_sector * BYTES_PER_SECTOR, os.SEEK_SET)
        return self.f.read(n_sectors * BYTES_PER_SECTOR)

    def write(self, data, i_sector):
        self.f.seek(i_sector * BYTES_PER_SECTOR, os.SEEK_SET)
        self.f.write(data)
        self.f.flush()

    def close(self):
        self.f.close()

    def show(self):
        from f6 import human_size
        print 'Using file {} of size {}B ({})'.format(
            self.image_fpath,
            len(self),
            human_size(len(self)),
        )
        print '{} sectors'.format(self.n_sectors)


if __name__ == '__main__':
    from conf import *
    import sys

    # test creation of image file
    with open('/dev/null', 'wb') as null:
        image_fpath = 'foo'
        h = Hardware(image_fpath)
        assert os.path.exists(image_fpath)
        os.system('rm {} && clear'.format(image_fpath))

    # test open existed image file
    image_fpath = IMAGE_FPATH
    assert os.path.exists(image_fpath)
    h = Hardware(image_fpath)
    h.show()
