import struct
import os

from util import print_hex, bytes_to_block
from conf import *
from hardware import Hardware
from bitmap import Bitmap
from meta import Meta
from fileentry import Directory, FileEntry, File


class FileSystem(object):

    def __init__(self, image_fpath, new=False):
        self.hardware = Hardware(image_fpath, new)
        self.init()

    def init(self):
        self.meta = meta = Meta(self.read_block(I_META))

        n_bytes_bitmap = meta.n_blocks_bitmap * BYTES_PER_BLOCK
        self.bitmap = Bitmap(self.read_block(I_BITMAP)[:n_bytes_bitmap])

        self.cur_dir = Directory(self, I_BITMAP + self.meta.n_blocks_bitmap)

    def format(self):
        self.new_bitmap()
        self.new_meta()
        self.new_root()

    def new_bitmap(self):
        self.bitmap = bitmap = Bitmap(self.hardware.n_sectors)

        n_blocks_bitmap = bytes_to_block(len(bitmap))
        bitmap[0:I_BITMAP+n_blocks_bitmap+1] = True

        self.write_block(bitmap.bytes, I_BITMAP)

    def new_meta(self):
        self.meta = meta = Meta()
        meta.n_sectors = self.hardware.n_sectors
        meta.n_blocks_bitmap = n_blocks_bitmap = bytes_to_block(len(self.bitmap))
        self.write_block(meta.bytes, I_META)

    def new_root(self):
        i_root = I_BITMAP + self.meta.n_blocks_bitmap
        self.cur_dir = root = Directory(self, 0)
        root.i_block = i_root
        self.write_block(root.bytes, i_root)

    def mkdir(self, name):
        cur = self.cur_dir
        if name in cur.children_names:
            return
        d = self.new_directory(name)
        cur.add_child(d)
        d.flush()

    def cd(self, name):
        cur = self.cur_dir
        if name == '..':
            if cur.i_parent:
                self.cur_dir = cur.parent
        else:
            child = cur.get_child_by_name(name)
            if child and child.is_dir:
                self.cur_dir = child

    def pwd(self):
        print self.cur_dir.fpath

    def ls(self):
        print self.cur_dir.children_names

    def touch(self, name):
        cur = self.cur_dir
        f = self.new_file(name)
        cur.add_child(f)
        f.flush()

    def exists(self, path):
        fpath = self.absolute_path(path)
        return bool(self.get_entry_by_fpath(fpath))

    def open(self, path):
        fpath = self.absolute_path(path)
        if not self.exists(fpath):
            self.touch(path.split('/')[-1])
        entry = self.get_entry_by_fpath(fpath)
        if not entry or entry.is_dir:
            return None
        else:
            return File(entry)

    def external_copy(self, src, dst):
        if not os.path.exists(src):
            print 'Source file "{}" does not exists!'.format(src)
            return
        f_dst = self.open(dst)
        with open(src, 'rb') as f_src:
            f_dst.write(f_src.read())

    def absolute_path(self, path):
        if path.startswith('/'):
            return path
        else:
            fpath = self.cur_dir.fpath
            if not fpath.endswith('/'):
                fpath += '/'
            fpath += path
            return fpath

    def get_entry_by_fpath(self, fpath):
        names = fpath.split('/')
        cur = self.cur_dir
        while cur and names:
            if cur.name == names[0] and len(names) == 1:
                return cur
            elif cur.is_dir:
                names = names[1:]
                if names:
                    cur = cur.get_child_by_name(names[0])
                else:
                    break
            else:
                break
        return None

    def alloc_block(self):
        i_block = self.bitmap.alloc_block()
        self.write_block(self.bitmap.bytes, I_BITMAP)
        return i_block

    def new_directory(self, name):
        d = Directory(self, 0)
        d.i_block = self.alloc_block()
        d.name = name
        return d

    def new_file(self, name):
        f = FileEntry(self, self.alloc_block(), new=True, name=name)
        return f

    def close(self):
        self.hardware.close()

    def write_block(self, data, i_block):
        self.hardware.write(data, i_block * SECTORS_PER_BLOCK)

    def read_block(self, i_block):
        return self.hardware.read(i_block * SECTORS_PER_BLOCK, SECTORS_PER_BLOCK)

    def show(self):
        self.meta.show()
        print

        print '=' * 40, 'bitmap'
        self.bitmap.show(32)
        print

        i_root = I_BITMAP + self.meta.n_blocks_bitmap
        for i in [1, 0] + range(i_root, i_root + 8):
            if i == 0:
                print
                continue
            print '=' * 40, 'block', i
            block_data = self.read_block(i)
            print_hex(block_data[:64])

        print
