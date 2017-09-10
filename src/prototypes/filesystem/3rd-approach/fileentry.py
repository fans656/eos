import struct

from entry import Entry
from conf import *


MAX_FILENAME_LENGTH = 474  # in bytes, not including the trailing zero
MAX_DATA_BLOCKS_PER_ENTRY = 895


class FileEntry(Entry):

    def __init__(self, disk, allocator, i_block=0, load=False, name='', file_size=0):
        if len(name) > MAX_FILENAME_LENGTH:
            raise Exception('file name too long, {} exceed the limit {}'.format(
                len(name), MAX_FILENAME_LENGTH))

        self.disk = disk
        self.allocator = allocator

        self.i_block = i_block
        if load:
            self.load()
        else:
            self.new(name)

    def load(self):
        data = self.disk.read_bytes(self.i_block * BYTES_PER_BLOCK, BYTES_PER_BLOCK)
        i = 0
        self.type = struct.unpack('<I', data[i:i+4])[0]
        i += 4
        self.i_parent = struct.unpack('<I', data[i:i+4])[0]
        i += 4
        self.i_prev_sibling = struct.unpack('<I', data[i:i+4])[0]
        i += 4
        self.i_next_sibling = struct.unpack('<I', data[i:i+4])[0]
        i += 4
        self.i_next_entry = struct.unpack('<I', data[i:i+4])[0]
        i += 4
        n_data_blocks = struct.unpack('<I', data[i:i+4])[0]
        i += 4
        self.file_size = struct.unpack('<Q', data[i:i+8])[0]
        i += 8
        name_len = struct.unpack('<I', data[i:i+4])[0]
        i += 4
        self.name = data[i:i+name_len]
        i = 512

        self.data_blocks = []
        for i in xrange(n_data_blocks):
            i_block = struct.unpack('<I', data[i*4:i*4+4])[0]
            self.data_blocks.append(i_block)

    @property
    def bytes(self):
        s = ''
        s += struct.pack('<I', self.type)
        s += struct.pack('<I', self.i_parent)
        s += struct.pack('<I', self.i_prev_sibling)
        s += struct.pack('<I', self.i_next_sibling)
        s += struct.pack('<I', self.i_next_entry)
        s += struct.pack('<I', len(self.data_blocks))
        s += struct.pack('<Q', self.file_size)
        s += struct.pack('<I', len(self.name))
        s += self.name + '\0' * (MAX_FILENAME_LENGTH - len(self.name) + 1)
        for i_block in self.data_blocks:
            s += struct.pack('<I', i_block)
        return s

    def new(self, name):
        self.type = TYPE_FILE
        self.i_parent = 0
        self.i_prev_sibling = 0
        self.i_next_sibling = 0
        self.i_next_entry = 0
        self.n_data_blocks = 0
        self.file_size = 0
        self.name = name
        self.data_blocks = []

    def flush(self):
        cur = self
        while cur:
            cur.disk.write_bytes(cur.i_block * BYTES_PER_BLOCK, cur.bytes)
            cur = cur.next_entry

    @property
    def next_entry(self):
        if self.i_next_entry:
            return FileEntry(self.disk, self.allocator, self.i_next_entry, load=True)
        else:
            return None

    def open(self):
        return File(self)

    def get_entry_by_i_block(self, i_block):
        from directory import Directory
        if i_block:
            d = Directory(self.disk, self.allocator, i_block, load=True)
            if d.is_dir:
                return d
            else:
                return FileEntry(self.disk, self.allocator, i_block,
                                 load=True)
        else:
            return None

    def read_bytes(self, i_byte, n_bytes):
        bytes_per_entry = MAX_DATA_BLOCKS_PER_ENTRY * BYTES_PER_BLOCK
        i_entry = i_byte // bytes_per_entry
        i_byte %= bytes_per_entry

        cur_entry = self.get_ith_entry(i_entry)
        block_index = i_byte // BYTES_PER_BLOCK
        i_byte_in_block = i_byte % BYTES_PER_BLOCK
        i_byte_in_data = 0
        res = ''
        while n_bytes:
            i_data_block = cur_entry.data_blocks[block_index]
            n_bytes_to_read = min(BYTES_PER_BLOCK - i_byte_in_block, n_bytes)
            res += self.disk.read_bytes(
                i_data_block * BYTES_PER_BLOCK + i_byte_in_block, n_bytes_to_read)
            i_byte_in_block = 0
            i_byte_in_data += n_bytes_to_read
            n_bytes -= n_bytes_to_read
            block_index += 1
            if block_index == len(cur_entry.data_blocks):
                block_index = 0
                cur_entry = cur_entry.next_entry
        return res

    def write_bytes(self, i_byte, data):
        n_bytes = len(data)
        new_size = i_byte + n_bytes
        if new_size > self.file_size:
            cur_n_data_blocks = self.file_size // BYTES_PER_BLOCK
            new_n_data_blocks = new_size // BYTES_PER_BLOCK
            more_n_data_blocks = new_n_data_blocks - cur_n_data_blocks
            self.last_entry.alloc(more_n_data_blocks)
            self.file_size = new_size
            self.flush()

        # now we have enough data blocks to put the new data
        bytes_per_entry = MAX_DATA_BLOCKS_PER_ENTRY * BYTES_PER_BLOCK
        i_entry = i_byte // bytes_per_entry
        i_byte %= bytes_per_entry

        cur_entry = self.get_ith_entry(i_entry)
        block_index = i_byte // BYTES_PER_BLOCK
        i_byte_in_block = i_byte % BYTES_PER_BLOCK
        i_byte_in_data = 0
        while n_bytes:
            i_data_block = cur_entry.data_blocks[block_index]
            n_bytes_to_write = min(
                BYTES_PER_BLOCK - i_byte_in_block, n_bytes)
            self.disk.write_bytes(
                i_data_block * BYTES_PER_BLOCK + i_byte_in_block,
                data[i_byte_in_data:i_byte_in_data+n_bytes_to_write])
            i_byte_in_block = 0
            i_byte_in_data += n_bytes_to_write
            n_bytes -= n_bytes_to_write
            block_index += 1
            if block_index == MAX_DATA_BLOCKS_PER_ENTRY:
                block_index = 0
                cur_entry.flush()
                cur_entry = cur_entry.next_entry
        cur_entry.flush()

    def expand_to_size(self, size):
        cur_n_data_blocks = (self.file_size + BYTES_PER_BLOCK - 1) // BYTES_PER_BLOCK
        new_n_data_blocks = (size + BYTES_PER_BLOCK - 1) // BYTES_PER_BLOCK
        more_n_data_blocks = new_n_data_blocks - cur_n_data_blocks
        print 'expand_to_size', self.i_block
        print 'self.i_block', self.i_block, 'self.i_next_entry', self.i_next_entry
        print 'self.last_entry.i_block', self.last_entry.i_block, self.last_entry.i_next_entry
        print 'more_n_data_blocks', more_n_data_blocks
        self.last_entry.alloc(more_n_data_blocks)
        self.file_size = size
        self.flush()

    def alloc(self, n_blocks):
        n_free_blocks = MAX_DATA_BLOCKS_PER_ENTRY - len(self.data_blocks)
        n_to_alloc = min(n_free_blocks, n_blocks)
        for _ in xrange(n_to_alloc):
            self.data_blocks.append(self.allocator.alloc())
        n_blocks -= n_to_alloc
        print 'alloc n_blocks', self.i_block, n_blocks
        if n_blocks > 0:
            self.new_next_entry()
            self.next_entry.alloc(n_blocks)
        self.flush()

    def new_next_entry(self):
        entry = FileEntry(
            self.disk, self.allocator, self.allocator.alloc(),
            file_size=self.file_size)
        entry.flush()
        print 'new_next_entry', entry.i_block
        self.i_next_entry = entry.i_block

    def get_ith_entry(self, i):
        cur = self
        for _ in xrange(i):
            cur = cur.next_entry
        return cur

    @property
    def last_entry(self):
        cur = self
        while cur.i_next_entry:
            cur = cur.next_entry
        return cur

    @property
    def n_entries(self):
        cur = self
        n = 0
        while cur:
            n += 1
            cur = cur.next_entry
        return n

    @property
    def n_blocks(self):
        cur = self
        n = 0
        while cur:
            n += len(cur.data_blocks)
            cur = cur.next_entry
        return n

class File(object):

    def __init__(self, entry):
        self.entry = entry
        self.size = entry.file_size
        self.disk = entry.disk
        self.allocator = entry.allocator
        self.data_blocks = self.entry.data_blocks
        self.pos = 0

    def seek(self, pos):
        if pos < 0 or pos > self.size:
            raise Exception('invalid pos {}, should be in [0, {})'.format(
                pos, self.size))
        self.pos = pos

    def tell(self):
        return self.pos

    def read(self, n_bytes):
        n_bytes = min(self.pos + n_bytes, self.size) - self.pos
        res = self.entry.read_bytes(self.pos, n_bytes)
        self.pos += n_bytes
        return res

    def write(self, data):
        self.entry.write_bytes(self.pos, data)
        self.pos += len(data)
        self.size = max(self.size, self.pos)
