import struct

from disk import Disk
from bitmap import Bitmap
from directory import Directory
from meta import Meta
from conf import *
from util import *


class FileSystem(object):

    def __init__(self, image_fpath=IMAGE_FPATH, new=False):
        self.image_fpath = image_fpath
        self.disk = disk = Disk(image_fpath=self.image_fpath)
        if new or disk.new:
            self.format()
        else:
            self.load()

    def format(self):
        self.meta = meta = Meta(self.disk, new=True)
        self.bitmap = Bitmap(self.disk, meta, new=True)

        self.root = root = Directory(self.disk, self.bitmap, meta.i_root)
        root.flush()

        self.load()

    def load(self):
        self.meta = meta = Meta(self.disk)
        self.bitmap = Bitmap(self.disk, meta)
        self.root = Directory(self.disk, self.bitmap, meta.i_root, load=True)
        self.cur = self.root

    def mkdir(self, name):
        cur = self.cur
        assert self.cur is self.root
        if name in cur.children_names:
            return
        cur.new_directory(name)

    def ls(self):
        print self.cur.children_names

    def cd(self, name):
        if name == '..':
            if self.cur.i_parent:
                self.cur = self.cur.parent
        else:
            child = self.cur.get_child_by_name(name)
            if child and child.is_dir:
                self.cur = child

    def show(self):
        meta = self.meta
        print '=' * 40, 'meta', meta.i_meta
        print_hex(self.disk.read_bytes(meta.i_meta * BYTES_PER_BLOCK, 64))
        print '=' * 40, 'bitmap', meta.i_bitmap
        print_hex(self.bitmap.bytes, 64)
        print '=' * 40, 'root', meta.i_root
        print_hex(self.disk.read_bytes(meta.i_root * BYTES_PER_BLOCK, 64))


if __name__ == '__main__':
    from util import *


    fs = FileSystem()
    fs.mkdir('foo')
    fs.ls()
    fs.cd('foo')
    fs.ls()
    fs.cd('..')
    fs.ls()
    fs.mkdir('bar')
