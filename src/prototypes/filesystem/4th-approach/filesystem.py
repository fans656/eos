import struct
import os

from disk import Disk
from entry import DirectoryEntry, FileEntry, File
from conf import *
from util import *


class Filesystem(object):

    def __init__(self, new=False):
        self.disk = Disk(new=new)
        if new:
            self.format()
        else:
            self.load()
        self.init()

    def ls(self):
        print [c.name for c in self.current_directory.children]

    def mkdir(self, path):
        dirpath = self.abspath(path)
        names = dirpath.split('/')[1:]
        cur = self.root_directory
        for name in names:
            child = cur.get_child_by_name(name)
            if not child:
                child = DirectoryEntry(self, self.alloc(), load=False)
                child.name = name
                cur.add_child(child)
            cur = child
        return cur

    def cd(self, name):
        if name == '..':
            if self.current_directory.i_parent:
                self.current_directory = self.current_directory.parent
        else:
            child = self.current_directory.get_child_by_name(name)
            if child:
                if child.is_dir:
                    self.current_directory = child
                else:
                    print 'Error: "{}" is not a directory'.format(name)
            else:
                print 'Error: "{}" not found'.format(name)

    def cat(self, path):
        f = self.open(path)
        print f.read(f.size)

    def touch(self, name):
        file_entry = FileEntry(self.disk, self.disk.alloc(), load=False)
        file_entry.name = name
        file_entry.flush()
        self.current_directory.add_child(file_entry)

    def tree(self):

        def show_tree(root, depth=0):
            indent = ' ' * 4 * depth
            print indent + root.name + ('/' if root.is_dir else '')
            if root.is_dir:
                for child in root.children:
                    show_tree(child, depth + 1)

        show_tree(self.root_directory)

    def open(self, path):
        fpath = self.abspath(path)
        dirpath = self.dirpath(fpath)
        name = self.basename(fpath)
        cur = self.mkdir(dirpath)
        child = cur.get_child_by_name(name)
        if not child:
            child = FileEntry(self, self.alloc(), load=False)
            child.name = name
            cur.add_child(child)
        cur = child
        return File(cur)

    def bundle_file(self, src, dst):
        with open(src, 'rb') as src:
            dst = self.open(dst)
            while True:
                data = src.read(4096)
                if not data:
                    break
                dst.write(data)

    def bundle_directory(self, src, dst='/'):
        src = os.path.abspath(src)
        for dirpath, dirs, fnames in os.walk(src):
            for fname in fnames:
                fpath = os.path.join(dirpath, fname)
                path = fpath[len(src)+1:]
                self.bundle_file(fpath, os.path.join(dst, path))

    def alloc(self):
        return self.bitmap.alloc()

    def free(self, i_block):
        self.bitmap[i_block] = False

    def read_block(self, i_block):
        return self.disk.read_block(i_block)

    def write_block(self, i_block, data):
        self.disk.write_block(i_block, data)

    def read_blocks(self, i_block, n_blocks):
        return self.disk.read_blocks(i_block, n_blocks)

    def write_blocks(self, i_block, data):
        self.disk.write_blocks(i_block, data)

    def read_bytes(self, i_byte, n_bytes):
        return self.disk.read_bytes(i_byte, n_bytes)

    def write_bytes(self, i_byte, data):
        self.disk.write_bytes(i_byte, data)

    def read_sector(self, i_sector):
        return self.disk.read_sector(i_sector)

    def write_sector(self, i_sector, data):
        return self.disk.write_sector(i_sector, data)

    def format(self):
        self.bitmap = Bitmap(self, I_BITMAP_BLOCK, self.disk.n_blocks, load=False)
        self.meta = Meta(self, self.alloc(), load=False)
        self.root_directory = DirectoryEntry(self, self.alloc(), load=False)
        self.flush()

    def load(self):
        self.meta = meta = Meta(self, I_META_BLOCK, load=True)
        self.bitmap = Bitmap(self, meta.i_bitmap, meta.n_blocks, load=True)
        self.root_directory = DirectoryEntry(self, self.meta.i_root, load=True)

    def init(self):
        self.current_directory = self.root_directory

    def abspath(self, path):
        if path.startswith('/'):
            return path
        dirpath = self.current_directory.fpath
        if not dirpath.endswith('/'):
            dirpath += '/'
        return dirpath + path

    def dirpath(self, fpath):
        return '/'.join(fpath.split('/')[:-1])

    def basename(self, fpath):
        return fpath.split('/')[-1]

    def flush(self):
        self.meta.flush()
        self.bitmap.flush()
        self.root_directory.flush()


class Meta(object):

    def __init__(self, filesystem, i_block, load):
        self.filesystem = filesystem
        self.i_block = i_block
        if load:
            self.load()
        else:
            self.new()
        self.init()

    def load(self):
        data = self.filesystem.read_block(self.i_block)
        i = 0
        self.n_blocks = struct.unpack('<I', data[i:i+4])[0]
        i += 4
        self.i_root = struct.unpack('<I', data[i:i+4])[0]
        i += 4

    def new(self):
        self.n_blocks = self.filesystem.disk.n_bytes // BYTES_PER_BLOCK
        n_bytes_of_bitmap = self.n_blocks // 8
        n_blocks_of_bitmap = (n_bytes_of_bitmap + BYTES_PER_BLOCK - 1) // BYTES_PER_BLOCK
        self.i_root = I_BITMAP_BLOCK + n_blocks_of_bitmap

    def init(self):
        self.i_meta = I_META_BLOCK
        self.i_bitmap = self.i_meta + 1
        self.n_blocks = self.filesystem.disk.n_blocks

    @property
    def bytes(self):
        s = ''
        s += struct.pack('<I', self.n_blocks)
        s += struct.pack('<I', self.i_root)
        return s

    def flush(self):
        self.filesystem.write_block(self.i_block, self.bytes)


class Bitmap(object):

    def __init__(self, filesystem, i_block, n_blocks, load):
        if n_blocks % (8 * 32) != 0:
            raise ValueError('the number of blocks should be a multiple of 256')
        self.filesystem = filesystem
        self.i_block = i_block
        self.n_blocks = n_blocks
        if load:
            self.load()
        else:
            self.new()

    def load(self):
        filesystem = self.filesystem
        meta = filesystem.meta
        data = filesystem.read_blocks(self.i_block, meta.i_root - meta.i_bitmap)
        data = data[:self.n_blocks // 8]
        self.a = [ord(val) for val in data]
        self.counts = [sum(bit_count(ord(v)) for v in data[i:i+32])
                       for i in xrange(0, len(data), 32)]
        self.size_in_blocks = (len(self.a) + BYTES_PER_BLOCK - 1) // BYTES_PER_BLOCK

    def new(self):
        n_blocks = self.n_blocks
        self.a = [0] * (n_blocks // 8)
        self.counts = [256] * (n_blocks // 8 // 32)
        self.size_in_blocks = (len(self.a) + BYTES_PER_BLOCK - 1) // BYTES_PER_BLOCK
        for i in xrange(N_RESERVED_BLOCKS):
            self[i] = True
        for i in xrange(self.i_block, self.i_block + self.size_in_blocks):
            self[i] = True

    def __getitem__(self, i_bit):
        i_byte = i_bit // 8
        i_bit = i_bit % 8
        return bool(self.a[i_byte] & (1 << i_bit))

    def __setitem__(self, i_bit, used):
        i_byte = i_bit // 8
        i_bit = i_bit % 8
        if used:
            self.a[i_byte] |= 1 << i_bit
            self.counts[i_byte // 32] -= 1
        else:
            self.a[i_byte] &= ~(1 << i_bit)
            self.counts[i_byte // 32] += 1
        self.filesystem.write_bytes(
            self.i_block * BYTES_PER_BLOCK + i_byte, chr(self.a[i_byte]))

    def alloc(self):
        for i, count in enumerate(self.counts):
            if count:
                break
        i_byte = i * 32
        for i_byte in xrange(i_byte, len(self.a)):
            if self.a[i_byte] != 0xff:
                for i_bit in xrange(8):
                    i = i_byte * 8 + i_bit
                    if not self[i]:
                        self[i] = True
                        return i
        else:
            raise RuntimeError('no more space')

    def flush(self):
        self.filesystem.write_blocks(self.i_block, self.bytes)

    @property
    def bytes(self):
        return ''.join(map(chr, self.a))

    def show(self, n_bytes=64):
        print '=' * 40, 'bitmap'
        print_hex(''.join(map(chr, self.a)), n_bytes)
        print '=' * 40, 'bitmap end'


if __name__ == '__main__':
    new = 0
    fs = Filesystem(new=new)
    disk = fs.disk
    if new:
        fs.bundle_directory('/home/fans656/eos/files')
        fs.tree()
    else:
        fs.tree()

        f = fs.open('snow-leopard.bmp')
        with open('t.bmp', 'wb') as dst:
            dst.write(f.read(f.size))
