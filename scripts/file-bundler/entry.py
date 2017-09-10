import struct

from conf import *


MAX_N_BYTES_NAME = 512 - 20
FILE_HEADER_LEN = 8 + 4 + 4
MAX_DATA_BLOCKS_PER_ENTRY = (4096 - 1024) // 4


class Entry(object):

    def __init__(self, filesystem, i_block, data=None):
        """
        Attributes:
            name - a string for the file's name

        Actual fields:
            uint32_t type
            uint32_t i_parent
            uint32_t i_prev_sibling
            uint32_t i_next_sibling
            uint32_t n_bytes_name
            char name[MAX_N_BYTES_NAME]
        """
        self.filesystem = filesystem
        self.i_block = i_block

        self.type = 0
        self.i_parent = 0
        self.i_prev_sibling = 0
        self.i_next_sibling = 0
        self.name = ''

        if data:
            self.load(data)

    @property
    def is_dir(self):
        return self.type == TYPE_DIRECTORY

    @property
    def fpath(self):
        names = []
        cur = self
        while cur:
            names.append(cur.name)
            cur = cur.parent
        s = '/'.join(reversed(names))
        if not s.startswith('/'):
            s = '/' + s
        return s

    def load(self, data):
        i = 0
        self.type = struct.unpack('<I', data[i:i+4])[0]
        i += 4
        self.i_parent = struct.unpack('<I', data[i:i+4])[0]
        i += 4
        self.i_prev_sibling = struct.unpack('<I', data[i:i+4])[0]
        i += 4
        self.i_next_sibling = struct.unpack('<I', data[i:i+4])[0]
        i += 4
        n_bytes_name = struct.unpack('<I', data[i:i+4])[0]
        i += 4
        self.name = data[i:i+n_bytes_name]
        i = 512
        return data[i:]

    @property
    def bytes(self):
        s = ''
        s += struct.pack('<I', self.type)
        s += struct.pack('<I', self.i_parent)
        s += struct.pack('<I', self.i_prev_sibling)
        s += struct.pack('<I', self.i_next_sibling)
        s += struct.pack('<I', len(self.name))
        s += self.name + '\0' * (MAX_N_BYTES_NAME - len(self.name))
        return s

    @property
    def parent(self):
        return self.get_entry_from_i_block(self.i_parent)

    @property
    def prev_sibling(self):
        return self.get_entry_from_i_block(self.i_prev_sibling)

    @property
    def next_sibling(self):
        return self.get_entry_from_i_block(self.i_next_sibling)

    def get_entry_from_i_block(self, i_block):
        if i_block == 0:
            return None
        data = self.filesystem.read_block(i_block)
        entry_type = struct.unpack('<I', data[:4])[0]
        if entry_type == TYPE_DIRECTORY:
            return DirectoryEntry(self.filesystem, i_block, data)
        elif entry_type == TYPE_FILE:
            return FileEntry(self.filesystem, i_block, data)
        else:
            raise Exception('Invalid entry type: {:02x}'.format(entry_type))

    def flush(self):
        self.filesystem.write_block(self.i_block, self.bytes)

    def __repr__(self):
        return '<{} {}>'.format(self.__class__.__name__, repr(self.name))


class DirectoryEntry(Entry):

    def __init__(self, filesystem, i_block, load):
        """
        Attributes:
            name - a string for the directory name

        Actual fields:
            uint32_t type
            uint32_t i_parent
            uint32_t i_prev_sibling
            uint32_t i_next_sibling
            uint32_t n_bytes_name
            char name[MAX_N_BYTES_NAME]
            uint32_t i_first_child
            uint32_t i_last_child
        """
        self.i_first_child = 0
        self.i_last_child = 0

        data = filesystem.read_block(i_block) if load else None
        super(DirectoryEntry, self).__init__(filesystem, i_block, data)

        self.type = TYPE_DIRECTORY

    @property
    def children(self):
        def iterator():
            cur = self.first_child
            while cur:
                yield cur
                cur = cur.next_sibling
        return iterator()

    @property
    def empty(self):
        return bool(self.i_first_child)

    def get_child_by_name(self, name):
        return next((c for c in self.children if c.name == name), None)

    def add_child(self, child):
        assert self.i_block != child.i_block

        child.i_parent = self.i_block
        if not self.empty:
            self.i_first_child = self.i_last_child = child.i_block
        else:
            last_child = self.last_child
            last_child.i_next_sibling = child.i_block
            child.i_prev_sibling = last_child.i_block
            self.i_last_child = child.i_block
            last_child.flush()
        self.flush()
        child.flush()

    def remove_child(self, child):
        self.take_child(child)
        child.free()

    def take_child(self, child):
        if child.i_prev_sibling:
            prev_sibling = child.prev_sibling
            prev_sibling.i_next_sibling = child.i_next_sibling
            prev_sibling.flush()
        if child.i_next_sibling:
            next_sibling = child.next_sibling
            next_sibling.i_prev_sibling = child.i_prev_sibling
            next_sibling.flush()
        if self.i_first_child == child.i_block:
            self.i_first_child = child.i_next_sibling
        if self.i_last_child == child.i_block:
            self.i_last_child = child.i_prev_sibling
        child.i_prev_sibling = child.i_next_sibling = child.i_parent = 0
        self.flush()

    def load(self, data):
        data = super(DirectoryEntry, self).load(data)
        i = 0
        self.i_first_child = struct.unpack('<I', data[i:i+4])[0]
        i += 4
        self.i_last_child = struct.unpack('<I', data[i:i+4])[0]
        i += 4

    def free(self):
        self.filesystem.free(self.i_block)

    @property
    def bytes(self):
        s = super(DirectoryEntry, self).bytes
        s += struct.pack('<I', self.i_first_child)
        s += struct.pack('<I', self.i_last_child)
        return s

    @property
    def first_child(self):
        return self.get_entry_from_i_block(self.i_first_child)

    @property
    def last_child(self):
        return self.get_entry_from_i_block(self.i_last_child)


class FileEntry(Entry):

    def __init__(self, filesystem, i_block, load):
        """
        Attributes:
            name - a string for the file name
            size - uint32_t for the file size in bytes

        Actual fields:
            uint32_t type
            uint32_t i_parent
            uint32_t i_prev_sibling
            uint32_t i_next_sibling
            uint32_t n_bytes_name
            char name[MAX_N_BYTES_NAME]
            uint64_t size
            uint32_t i_next_entry
            char reserved[512 - FILE_HEADER_LEN]
        """
        self.size = 0
        self.i_next_entry = 0
        self.data_blocks = []

        data = filesystem.read_block(i_block) if load else None
        super(FileEntry, self).__init__(filesystem, i_block, data)

        self.type = TYPE_FILE

    def reserve(self, size):
        if size <= self.size:
            return
        n_bytes = size - self.size
        cur = self.last_entry
        # alloc in last block
        if cur.data_blocks and cur.size % BYTES_PER_BLOCK:
            free_of_last_block = BYTES_PER_BLOCK - cur.size % BYTES_PER_BLOCK
            n_alloc = min(free_of_last_block, n_bytes)
            n_bytes -= n_alloc
        # alloc by new block or even new entry
        while n_bytes > 0:
            n_free_blocks = MAX_DATA_BLOCKS_PER_ENTRY - len(cur.data_blocks)
            for _ in xrange(n_free_blocks):
                cur.data_blocks.append(cur.filesystem.alloc())
                n_bytes -= BYTES_PER_BLOCK
                if n_bytes <= 0:
                    break
            cur.flush()
            if n_bytes > 0:
                cur.new_file_entry()
                cur.flush()
                cur = cur.next_entry
        self.size = size
        self.flush()

    def get_data_block_by_pos(self, pos):
        bytes_per_entry = BYTES_PER_BLOCK * MAX_DATA_BLOCKS_PER_ENTRY
        i_entry = pos // bytes_per_entry
        pos %= bytes_per_entry
        i_block = pos // BYTES_PER_BLOCK
        i_byte = pos % BYTES_PER_BLOCK
        return DataBlock(self.get_ith_entry(i_entry), i_block, i_byte)

    def get_ith_entry(self, i):
        cur = self
        for _ in xrange(i):
            cur = cur.next_entry
        return cur

    def new_file_entry(self):
        entry = FileEntry(self.filesystem, self.filesystem.alloc(), load=False)
        self.i_next_entry = entry.i_block
        entry.flush()

    @property
    def last_entry(self):
        cur = self
        while cur.i_next_entry:
            cur = cur.next_entry
        return cur

    @property
    def next_entry(self):
        if self.i_next_entry:
            return FileEntry(self.filesystem, self.i_next_entry, load=True)
        else:
            return None

    @property
    def entries(self):
        res = []
        cur = self
        while cur:
            res.append(cur)
            cur = cur.next_entry
        return res

    def load(self, data):
        data = super(FileEntry, self).load(data)
        i = 0
        self.size = struct.unpack('<Q', data[i:i+8])[0]
        i += 8
        self.i_next_entry = struct.unpack('<I', data[i:i+4])[0]
        i += 4
        n_data_blocks = struct.unpack('<I', data[i:i+4])[0]
        i = 512

        data_blocks = []
        for _ in xrange(n_data_blocks):
            i_block = struct.unpack('<I', data[i:i+4])[0]
            data_blocks.append(i_block)
            i += 4
        self.data_blocks = data_blocks

    @property
    def bytes(self):
        s = super(FileEntry, self).bytes
        s += struct.pack('<Q', self.size)
        s += struct.pack('<I', self.i_next_entry)
        s += struct.pack('<I', len(self.data_blocks))
        s += '\0' * (512 - FILE_HEADER_LEN)
        for i_block in self.data_blocks:
            s += struct.pack('<I', i_block)
        return s


class DataBlock(object):

    def __init__(self, entry, i_block, i_byte):
        self.entry = entry
        self.i_block = i_block
        self.i_byte = i_byte

    def read(self, n_bytes):
        n_read = min(BYTES_PER_BLOCK - self.i_byte, n_bytes)
        return self.entry.filesystem.read_bytes(self.offset, n_read)

    def write(self, data, i):
        n = len(data) - i
        n_write = min(n, BYTES_PER_BLOCK - self.i_byte)
        self.entry.filesystem.write_bytes(self.offset, data[i:i+n_write])
        return i + n_write

    def next(self):
        i_block = self.i_block + 1
        entry = self.entry
        if i_block == MAX_DATA_BLOCKS_PER_ENTRY:
            i_block = 0
            entry = entry.next_entry
        return DataBlock(entry, i_block, 0)

    @property
    def offset(self):
        i_data_block = self.entry.data_blocks[self.i_block]
        return i_data_block * BYTES_PER_BLOCK + self.i_byte


class File(object):

    def __init__(self, entry):
        self.entry = entry
        self.pos = 0

    def tell(self):
        return self.pos

    def seek(self, pos):
        if pos < 0:
            raise ValueError('seek pos can not be negative')
        self.pos = pos

    @property
    def size(self):
        return self.entry.size

    def read(self, n_bytes):
        if self.pos >= self.size or n_bytes <= 0:
            return ''
        block = self.entry.get_data_block_by_pos(self.pos)
        res = ''
        while n_bytes:
            s = block.read(n_bytes)
            res += s
            n_bytes -= len(s)
            block = block.next()
        return res

    def write(self, data):
        n_bytes = len(data)
        self.entry.reserve(self.pos + n_bytes)
        e = self.entry
        if n_bytes:
            block = self.entry.get_data_block_by_pos(self.pos)
            i_byte_data = 0
            while i_byte_data < len(data):
                i_byte_data = block.write(data, i_byte_data)
                block = block.next()
        self.pos += n_bytes


if __name__ == '__main__':
    from util import *


    class MockFilesystem(object):

        def __init__(self):
            self.entries = []

        def read_block(self, i):
            i -= 1
            if not 0 <= i < len(self.entries):
                raise IndexError()
            return self.entries[i].bytes

        def write_block(self, i, data):
            i -= 1
            if not 0 <= i < len(self.entries):
                raise IndexError()
            self.entries[i].load(data)

        def new_directory(self, name):
            return self.new_entry(DirectoryEntry, name)

        def new_file(self, name):
            return self.new_entry(FileEntry, name)

        def new_entry(self, cls, name):
            e = cls(self, len(self.entries) + 1, load=False)
            e.name = name
            self.entries.append(e)
            return e

        def free(self, i):
            pass

        def alloc(self):
            self.entries.append('\0' * 4096)
            return len(self.entries) - 1


    def is_root(e):
        return e.prev_sibling == e.next_sibling == e.parent == None


    def same_entry(e1, e2):
        return e1.i_block == e2.i_block


    def children_names(directory):
        return [c.name for c in directory.children]


    def show_tree(root, depth=0):
        indent = ' ' * 4 * depth
        print indent + root.name + ('/' if root.is_dir else '')
        if root.is_dir:
            for child in root.children:
                show_tree(child, depth + 1)


    # Tree structure test
    # with the following arrangement
    '''
    /
        doc/
            empty/
            t.txt
        pic/
            girl.jpg
            hidden/
            me.png
        single/
            foo
    '''

    filesystem = MockFilesystem()

    root = filesystem.new_directory('')

    doc = filesystem.new_directory('doc')
    empty = filesystem.new_directory('empty')
    t_txt = filesystem.new_file('t.txt')

    pic = filesystem.new_directory('pic')
    girl_jpg = filesystem.new_file('girl.jpg')
    hidden = filesystem.new_directory('hidden')
    me_png = filesystem.new_file('me.png')

    single = filesystem.new_directory('single')
    foo = filesystem.new_file('foo')

    assert root.first_child == root.last_child == None

    assert doc.first_child == doc.last_child == doc.prev_sibling == doc.next_sibling == None
    doc.add_child(empty)
    assert doc.first_child.name == doc.last_child.name == 'empty'
    assert same_entry(empty.parent, doc)
    doc.add_child(t_txt)
    assert same_entry(empty.next_sibling, t_txt)
    assert same_entry(empty, t_txt.prev_sibling)
    assert same_entry(empty.parent, doc)
    assert same_entry(t_txt.parent, doc)
    root.add_child(doc)

    pic.add_child(girl_jpg)
    pic.add_child(hidden)
    pic.add_child(me_png)
    root.add_child(pic)

    assert same_entry(girl_jpg.next_sibling, hidden)
    assert same_entry(hidden.next_sibling, me_png)
    assert me_png.next_sibling is None

    assert same_entry(me_png.prev_sibling, hidden)
    assert same_entry(hidden.prev_sibling, girl_jpg)
    assert girl_jpg.prev_sibling is None

    assert root.last_child.name == 'pic'
    assert root.first_child.next_sibling.name == 'pic'

    single.add_child(foo)
    root.add_child(single)

    assert root.last_child.name == 'single'
    assert root.first_child.next_sibling.name == 'pic'


    root = DirectoryEntry(filesystem, 1, load=True)
    assert is_root(root)

    doc = root.first_child
    assert doc.name == 'doc'
    assert doc.prev_sibling is None

    pic = root.first_child.next_sibling
    assert pic.name == 'pic'
    pic = root.last_child.prev_sibling
    assert pic.name == 'pic'

    single = root.last_child
    assert single.name == 'single'
    assert same_entry(single.first_child, single.last_child)


    #show_tree(root)
    #print
    #root.take_child(pic)
    #show_tree(root)
    #print
    #empty.add_child(pic)
    #show_tree(root)


    # File read/write test
    from filesystem import Filesystem
    from f6 import human_size

    filesystem = Filesystem()
    fe = FileEntry(filesystem, 1, load=False)
    fe.name = 't.txt'

    f = File(fe)
    #s = 'hello'
    #f.write(s)
    #f.seek(0)
    #assert f.read(len(s)) == s

    #f.seek(2)
    #f.write('__')
    #f.seek(0)
    #assert f.read(len(s)) == 'he__o'

    #f.seek(0)
    ##f.write('a' * 4096)
    #f.entry.reserve(4096)
    #f.seek(0)
    #f.write('a' * 4097)


    import random
    for size in [
            5, 4096, 4097, 4096 * MAX_DATA_BLOCKS_PER_ENTRY + 1
    ]:
        s = ''.join(chr(ord('a') + random.randint(0, 25)) for _ in xrange(size))
        f.seek(0)
        f.write(s)
        f.seek(0)
        assert f.read(len(s)) == s
        print size, 'ok'
