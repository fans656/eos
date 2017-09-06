import os
import struct


KB = 1024
MB = 1024 * KB

IMAGE_FPATH = 'disk.img'
BYTES_PER_SECTOR = 512
MAX_N_FILES = 1024
N_BYTES_FILE_ENTRY = 64
I_FILE_ENTRIES_SECTOR = 1 * MB / BYTES_PER_SECTOR
I_FILE_ENTRIES_END_SECTOR = (
    I_FILE_ENTRIES_SECTOR + (
        MAX_N_FILES * N_BYTES_FILE_ENTRY + BYTES_PER_SECTOR - 1
    ) // BYTES_PER_SECTOR)


def gen_disk_file(fpath):
    with open(fpath, 'wb') as f:
        for _ in xrange(16):
            f.write('\x00' * MB)


class FileSystem(object):

    def __init__(self, image_fpath):
        if not os.path.exists(image_fpath):
            gen_disk_file(image_fpath)
        self.f = open(image_fpath, 'rb+')
        self.init()

    def init(self):
        self.f.seek(I_FILE_ENTRIES_SECTOR)
        data = self.f.read(MAX_N_FILES * N_BYTES_FILE_ENTRY)
        self.file_entries = file_entries = []
        for i in xrange(MAX_N_FILES):
            base = i * N_BYTES_FILE_ENTRY
            file_entries.append(FileEntry(data[base:base+N_BYTES_FILE_ENTRY]))
        self.f.seek(0, os.SEEK_END)
        self.i_end_sector = self.f.tell() // BYTES_PER_SECTOR

    def format(self):
        self.seek(I_FILE_ENTRIES_SECTOR)
        for _ in xrange(MAX_N_FILES):
            self.f.write(FileEntry().bytes)

    def ls(self):
        print [e for e in self.file_entries if e]

    def open(self, fname):
        entry = next((File(e) for e in self.file_entries if e), None)
        if not entry:
            entry = self.new_entry(fname)
        return File(self.f, entry)

    def new_entry(self, fname):
        entry = FileEntry()
        entry.i_sector = self.i_avail_sector
        entry.fname = fname
        entry.flush(self.f, self.i_avail_entry)
        return entry

    @property
    def i_avail_entry(self):
        return next(i for i, e in enumerate(self.file_entries) if not e)

    @property
    def i_avail_sector(self):
        a = [(e.i_sector, e.i_end_sector) for e in self.file_entries]
        a.extend([
            (I_FILE_ENTRIES_END_SECTOR, I_FILE_ENTRIES_END_SECTOR),
            (self.i_end_sector, self.i_end_sector),
        ])
        a.sort(key=lambda (b, _): b)
        res_beg = res_end = None
        for i in xrange(len(a) - 1):
            beg = a[i][1]
            end = a[i + 1][0]
            if res_beg is None or end - beg > res_end - res_beg:
                res_beg, res_end = beg, end
        return res_beg

    def read(self, i_sector, count):
        self.seek(i_sector)
        return self.f.read(count * BYTES_PER_SECTOR)

    def write(self, data, i_sector):
        n = len(data)
        data += '\x00' * (BYTES_PER_SECTOR - n % BYTES_PER_SECTOR)
        self.seek(i_sector)
        self.f.write(data)

    def seek(self, i_sector):
        self.f.seek(i_sector * BYTES_PER_SECTOR, os.SEEK_SET)


class FileEntry(object):

    def __init__(self, data=None):
        self.i_sector = 0
        self.n_bytes = 0
        self.fname = ''
        if data:
            self.i_sector = struct.unpack('<I', data[:4])[0]
            self.n_bytes = struct.unpack('<I', data[4:8])[0]
            fname = data[8:]
            self.fname = fname[:fname.index('\0')]

    @property
    def bytes(self):
        s = ''
        s += struct.pack('<I', self.i_sector)
        s += struct.pack('<I', self.n_bytes)
        s += self.fname + '\0'
        s += '\0' * (N_BYTES_FILE_ENTRY - len(s))
        return s

    @property
    def i_end_sector(self):
        return self.i_sector + (self.n_bytes + BYTES_PER_SECTOR - 1) // BYTES_PER_SECTOR

    def flush(self, f, i_entry):
        f.seek(
            I_FILE_ENTRIES_SECTOR * BYTES_PER_SECTOR + i_entry * N_BYTES_FILE_ENTRY,
            os.SEEK_SET)
        f.write(self.bytes)

    def __nonzero__(self):
        return self.i_sector != 0


class File(object):

    def __init__(self, f, entry):
        self.f = f
        self.entry = entry
        self.pos = self.entry.i_sector * BYTES_PER_SECTOR

    def read(self, n_bytes):
        self.f.seek(self.pos, os.SEEK_SET)
        self.f.read(n_bytes)

    def write(self, data):
        self.f.seek(self.pos, os.SEEK_SET)
        self.f.write(data)
        self.pos += len(data)


fs = FileSystem(IMAGE_FPATH)
fs.ls()
f = fs.open('foo')
print f.entry.i_sector
