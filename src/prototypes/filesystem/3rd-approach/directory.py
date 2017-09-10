import struct

from entry import Entry
from fileentry import FileEntry, File
from conf import *


class Directory(Entry):

    def __init__(self, disk, allocator, i_block=0, load=False):
        self.disk = disk
        self.allocator = allocator

        self.i_block = i_block
        if load:
            self.load()
        else:
            self.new()

    def load(self):
        data = self.disk.read_bytes(self.i_block * BYTES_PER_BLOCK, BYTES_PER_SECTOR)
        i = 0
        self.type = struct.unpack('<I', data[i:i+4])[0]
        i += 4
        self.i_parent = struct.unpack('<I', data[i:i+4])[0]
        i += 4
        self.i_prev_sibling = struct.unpack('<I', data[i:i+4])[0]
        i += 4
        self.i_next_sibling = struct.unpack('<I', data[i:i+4])[0]
        i += 4
        self.i_first_child = struct.unpack('<I', data[i:i+4])[0]
        i += 4
        self.i_last_child = struct.unpack('<I', data[i:i+4])[0]
        i += 4
        name_len = struct.unpack('<I', data[i:i+4])[0]
        i += 4
        self.name = data[i:i+name_len]
        i += name_len

    @property
    def bytes(self):
        s = ''
        s += struct.pack('<I', self.type)
        s += struct.pack('<I', self.i_parent)
        s += struct.pack('<I', self.i_prev_sibling)
        s += struct.pack('<I', self.i_next_sibling)
        s += struct.pack('<I', self.i_first_child)
        s += struct.pack('<I', self.i_last_child)
        s += struct.pack('<I', len(self.name))
        s += self.name + '\0'
        return s

    def new(self):
        self.type = TYPE_DIRECTORY
        self.i_parent = 0
        self.i_first_child = 0
        self.i_last_child = 0
        self.i_prev_sibling = 0
        self.i_next_sibling = 0
        self.name = ''

    def flush(self):
        self.disk.write_bytes(self.i_block * BYTES_PER_BLOCK, self.bytes)

    def free(self):
        for child in self.children:
            child.free()
        self.allocator.free(self.i_block)

    def new_directory(self, name):
        child = Directory(self.disk, self.allocator, self.allocator.alloc())
        child.name = name
        self.add_child(child)
        return child

    def new_file(self, name):
        child = FileEntry(self.disk, self.allocator, self.allocator.alloc(), name=name)
        self.add_child(child)
        return child

    def add_child(self, child):
        tail = self.last_child
        if tail:
            tail.i_next_sibling = child.i_block
            child.i_prev_sibling = tail.i_block
            child.i_next_sibling = 0
            self.i_last_child = child.i_block
            tail.flush()
        else:
            self.i_first_child = self.i_last_child = child.i_block
            child.i_prev_sibling = child.i_next_sibling = 0
        child.i_parent = self.i_block
        self.flush()
        child.flush()

    def remove_child_by_name(self, name):
        child = self.get_child_by_name(name)
        if child:
            self.remove_child(child)

    def move_child_to_new_parent(self, child, parent):
        self.take_child(child)
        parent.add_child(child)

    def take_child(self, child):
        if self.i_first_child == child.i_block:
            self.i_first_child = child.i_next_sibling
        if self.i_last_child == child.i_block:
            self.i_last_child = child.i_prev_sibling
        prev_sibling = child.prev_sibling
        if prev_sibling:
            prev_sibling.i_next_sibling = child.i_next_sibling
            prev_sibling.flush()
        next_sibling = child.next_sibling
        if next_sibling:
            next_sibling.i_prev_sibling = child.i_prev_sibling
            next_sibling.flush()
        self.flush()

    def remove_child(self, child):
        self.take_child(child)
        child.free()

    def rename(self, name):
        self.name = name
        self.flush()

    @property
    def children(self):
        def children_iter():
            child = self.first_child
            while child:
                yield child
                child = child.next_sibling
        return children_iter()

    @property
    def children_names(self):
        return [c.name for c in self.children]

    @property
    def first_child(self):
        return self.get_entry_by_i_block(self.i_first_child)

    @property
    def last_child(self):
        return self.get_entry_by_i_block(self.i_last_child)

    def get_entry_by_i_block(self, i_block):
        if i_block:
            d = Directory(self.disk, self.allocator, i_block, load=True)
            if d.is_dir:
                return d
            else:
                return FileEntry(self.disk, self.allocator, i_block, load=True)
        else:
            return None

    def get_entry_by_fpath(self, fpath):
        names = fpath[1:].split('/')
        cur = self
        for name in names:
            child = cur.get_child_by_name(name)
            if not child:
                return None
            cur = child
            if not cur:
                return None
        return cur

    def get_child_by_name(self, name):
        return next((c for c in self.children if c.name == name), None)


if __name__ == '__main__':
    from disk import Disk
    from bitmap import Bitmap
    from meta import Meta
    from util import *


    #if os.path.exists(IMAGE_FPATH):
    #    os.system('rm {}'.format(IMAGE_FPATH))
    disk = Disk()
    bitmap = Bitmap(disk, Meta(disk, new=True), new=True)

    #print_hex(disk.read_bytes(258 * BYTES_PER_BLOCK, 32))
    d = Directory(disk, bitmap, 258)
    d.flush()
    print
    print 'Newly', d.fpath
    print_hex(disk.read_bytes(258 * BYTES_PER_BLOCK, 32))
    bitmap.show()

    #d = Directory(disk, bitmap, 258)
    #print_hex(disk.read_bytes(258 * BYTES_PER_BLOCK, 32))

    d.new_directory('foo')
    print
    print 'Create', d.get_child_by_name('foo').fpath
    print_hex(disk.read_bytes(258 * BYTES_PER_BLOCK, 32))
    bitmap.show()

    d.new_directory('bar')
    print
    print 'Create', d.get_child_by_name('bar').fpath
    print_hex(disk.read_bytes(258 * BYTES_PER_BLOCK, 32))
    bitmap.show()

    d.new_directory('baz')
    print
    print 'Create', d.get_child_by_name('baz').fpath
    print_hex(disk.read_bytes(258 * BYTES_PER_BLOCK, 32))
    bitmap.show()

    #d.remove_child_by_name('foo')
    #d.remove_child_by_name('bar')
    #d.remove_child_by_name('baz')
    #print [c.name for c in d.children]
    #bitmap.show()

    d.move_child_to_new_parent(d.get_child_by_name('foo'), d.get_child_by_name('bar'))
    print d.children_names
    print d.get_child_by_name('bar').children_names
    d.get_child_by_name('baz').rename('baaaaaaaaz')
    print d.children_names
