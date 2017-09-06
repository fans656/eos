import os
import struct

from conf import *


class Entry(object):

    def flush(self):
        self.filesystem.write_block(self.bytes, self.i_block)

    @property
    def parent(self):
        return file_entry(self.filesystem, self.i_parent)

    @property
    def next_sibling(self):
        return file_entry(self.filesystem, self.i_next_sibling)

    @property
    def fpath(self):
        names = []
        cur = self
        while cur:
            names.append(cur.name)
            cur = cur.parent
        s = '/'.join(reversed(names))
        if not s:
            s = '/'
        return s


class Directory(Entry):

    def __init__(self, filesystem, i_block):
        self.filesystem = filesystem
        self.i_block = i_block
        self.is_dir = True
        self.i_parent = 0
        self.i_next_sibling = 0
        self.i_first_child = 0
        self.name = ''

        if i_block:
            data = filesystem.read_block(i_block)
            i = 0
            self.is_dir = struct.unpack('<I', data[i:i+4])[0]
            i += 4
            self.i_parent = struct.unpack('<I', data[i:i+4])[0]
            i += 4
            self.i_next_sibling = struct.unpack('<I', data[i:i+4])[0]
            i += 4
            self.i_first_child = struct.unpack('<I', data[i:i+4])[0]
            i += 4
            name_len = struct.unpack('<I', data[i:i+4])[0]
            i += 4
            self.name = data[i:i+name_len]

    @property
    def bytes(self):
        s = ''
        s += struct.pack('<I', self.is_dir)
        s += struct.pack('<I', self.i_parent)
        s += struct.pack('<I', self.i_next_sibling)
        s += struct.pack('<I', self.i_first_child)
        s += struct.pack('<I', len(self.name))
        s += self.name
        return s

    @property
    def first_child(self):
        return file_entry(self.filesystem, self.i_first_child)

    @property
    def children_names(self):
        res = []
        cur = self.first_child
        while cur:
            res.append(cur.name)
            cur = cur.next_sibling
        return res

    @property
    def last_child(self):
        cur = self.first_child
        while cur and cur.i_next_sibling:
            cur = cur.next_sibling
        return cur

    def get_child_by_name(self, name):
        cur = self.first_child
        while cur:
            if cur.name == name:
                return cur
            cur = cur.next_sibling
        return None

    def add_child(self, child):
        child.i_parent = self.i_block
        last_child = self.last_child
        if last_child:
            last_child.i_next_sibling = child.i_block
            last_child.flush()
        else:
            self.i_first_child = child.i_block
            self.flush()


class FileEntry(Entry):

    def __init__(self, filesystem, i_block, new=False, name=''):
        self.filesystem = filesystem
        self.i_block = i_block
        self.is_dir = False
        self.i_parent = 0
        self.i_next_sibling = 0
        self.prev_block = 0
        self.i_next_block = 0
        self.i_data_block = 0
        self.file_size = 0
        self.name = name

        if i_block and not new:
            data = filesystem.read_block(i_block)
            i = 0
            self.is_dir = struct.unpack('<I', data[i:i+4])[0]
            i += 4
            self.i_parent = struct.unpack('<I', data[i:i+4])[0]
            i += 4
            self.i_next_sibling = struct.unpack('<I', data[i:i+4])[0]
            i += 4
            self.i_next_block = struct.unpack('<I', data[i:i+4])[0]
            i += 4
            self.i_data_block = struct.unpack('<I', data[i:i+4])[0]
            i += 4
            self.file_size = struct.unpack('<Q', data[i:i+8])[0]
            i += 8
            name_len = struct.unpack('<I', data[i:i+4])[0]
            i += 4
            self.name = data[i:i+name_len]

        if new:
            self.i_data_block = self.filesystem.alloc_block()

    @property
    def bytes(self):
        s = ''
        s += struct.pack('<I', self.is_dir)
        s += struct.pack('<I', self.i_parent)
        s += struct.pack('<I', self.i_next_sibling)
        s += struct.pack('<I', self.i_next_block)
        s += struct.pack('<I', self.i_data_block)
        s += struct.pack('<Q', self.file_size)
        s += struct.pack('<I', len(self.name))
        s += self.name
        return s

    def new_next_block(self):
        i_block = self.filesystem.alloc_block()
        self.i_next_block = i_block
        e = FileEntry(self.filesystem, i_block, new=True)
        e.flush()
        self.flush()
        return i_block

    @property
    def next_block(self):
        return file_entry(self.filesystem, self.i_next_block)


class File(object):

    def __init__(self, entry):
        self.head_entry = entry
        self.filesystem = entry.filesystem
        self.pos = 0
        self.size = entry.file_size

    def read(self, n_bytes=None):
        if n_bytes is None:
            n_bytes = self.size
        if self.pos + n_bytes > self.size:
            n_bytes = self.size - self.pos
        res = ''
        cur = self.cur_entry
        while n_bytes:
            block_data = self.filesystem.read_block(cur.i_data_block)
            i_to_take = self.pos % BYTES_PER_BLOCK
            n_take = min(BYTES_PER_BLOCK - i_to_take, n_bytes)
            res += block_data[i_to_take:i_to_take+n_take]
            n_bytes -= n_take
            self.pos += n_take
            cur = cur.next_block
        return res

    def write(self, data):
        while data:
            cur = self.cur_entry
            block_data = self.filesystem.read_block(cur.i_data_block)
            n_left = self.pos % BYTES_PER_BLOCK
            n_new = min(BYTES_PER_BLOCK - n_left, len(data))
            n_right = max(BYTES_PER_BLOCK - n_left - n_new, 0)
            block_data = block_data[:n_left] + data[:n_new] + block_data[-n_right:]
            self.filesystem.write_block(block_data, cur.i_data_block)
            data = data[n_new:]
            self.pos += n_new
            self.size = max(self.size, self.pos)
        self.head_entry.file_size = self.size
        self.head_entry.flush()

    def seek(self, offset, anchor=os.SEEK_SET):
        if anchor == os.SEEK_SET:
            self.pos = offset
        else:
            raise Exception('not supported')
        if not 0 <= offset <= self.size:
            raise Exception('invalid seek position')
        self.pos = offset

    def tell(self):
        return self.pos

    @property
    def cur_entry(self):
        """
        get the block entry correspond to current pos
        """
        end = BYTES_PER_BLOCK
        cur = self.head_entry
        while end < self.pos:
            cur = cur.next_block
            end += BYTES_PER_BLOCK
        if end == self.pos:
            cur.new_next_block()
            cur = cur.next_block
        return cur


def file_entry(filesystem, i_block):
    if i_block == 0:
        return None
    data = filesystem.read_block(i_block)
    is_dir = struct.unpack('<I', data[:4])[0]
    if is_dir:
        return Directory(filesystem, i_block)
    else:
        return FileEntry(filesystem, i_block)
