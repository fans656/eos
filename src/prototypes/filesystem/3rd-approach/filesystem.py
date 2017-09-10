import struct

from disk import Disk
from bitmap import Bitmap
from directory import Directory
from fileentry import File
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

    def bundle(self, fpath):
        pass

    def touch(self, name):
        self.cur.new_file(name)

    def exists(self, path):
        fpath = self.absolute_path(path)
        entry = self.root.get_entry_by_fpath(fpath)
        return bool(entry)

    def open(self, path):
        fpath = self.absolute_path(path)
        entry = self.root.get_entry_by_fpath(fpath)
        if not entry:
            raise Exception('file not found')
        if entry.is_dir:
            raise Exception('can not open directory')
        return File(entry)

    def absolute_path(self, path):
        if path.startswith('/'):
            return path
        else:
            s = self.cur.fpath
            if not s.endswith('/'):
                s += '/'
            s += path
            return s

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
    fs.format()
    #fs.mkdir('foo')
    #fs.ls()
    #fs.cd('foo')
    #fs.ls()
    #fs.cd('..')
    #fs.ls()
    #fs.mkdir('bar')
    #fs.ls()

    bitmap = fs.bitmap
    #print 'Initial format'
    #bitmap.show(64)

    fs.touch('t.txt')
    f = fs.open('/t.txt')
    e = f.entry
    #assert e.n_entries == 1
    #assert e.n_blocks == 0

    #print
    #print 'After touch t.txt'
    #bitmap.show(64)

    #e.expand_to_size(4096)
    #assert e.n_entries == 1
    #assert e.n_blocks == 1
    #print
    #print 'After expand to 4096'
    #bitmap.show(64)

    #e.expand_to_size(4097)
    #assert e.n_entries == 1
    #assert e.n_blocks == 2
    #print
    #print 'After expand to 4097'

    size = 4 * KB * 896
    e.expand_to_size(size)
    #print e.n_entries
    #print e.n_blocks
    exit()
    assert e.n_entries == 3
    assert e.n_blocks == 895 * 3
    print
    print 'After expand to {}'.format(size)
    bitmap.show(512)
    exit()

    #fs.bitmap.show()
    print 'Disk size', fs.meta.n_blocks * BYTES_PER_BLOCK
    print bitmap.size_in_bytes
    print bitmap.size_in_blocks

    #e.expand_to_size(4 * KB * 252)
    #fs.bitmap.show()
    #assert e.n_entries == 1
    #assert e.n_blocks == 894
