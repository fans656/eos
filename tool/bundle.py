#!/usr/bin/env python
# bundle a directory recursively into the disk image
import os
import struct


BPB = 4096
RESERVED_BLOCKS = 256
BITMAP_BLOCKS = 8
ROOT_BLOCK = RESERVED_BLOCKS + BITMAP_BLOCKS
FILES_BLOCK = ROOT_BLOCK + 1
MAX_NAME_LEN = 256


def write_entry(block, fpath, has_next, i_data_block):
    name = fpath[len(base):]
    img.seek(block * BPB)
    img.write(struct.pack('<I', block))
    img.write(struct.pack('<I', block + 1 if has_next else 0))
    img.write(name + '\0' * (MAX_NAME_LEN - len(name)))
    size = os.stat(fpath).st_size
    img.write(struct.pack('<I', size))
    img.write(struct.pack('<I', 1))  # n_blocks

    # just 1 BlocksRange
    img.write(struct.pack('<I', i_data_block))  # BlocksRange beg
    blocks = (size + BPB - 1) // BPB
    img.write(struct.pack('<I', blocks))  # BlocksRange count
    with open(fpath, 'rb') as f:
        img.seek(i_data_block * BPB)
        img.write(f.read())
    return i_data_block + blocks


img = open('bin/eos.img', 'rb+')

fpaths = []
base = 'files'
for dirpath, dirs, fnames in os.walk(base):
    for fname in fnames:
        fpath = os.path.join(dirpath, fname)
        fpaths.append(fpath)

i_data_block = FILES_BLOCK + len(fpaths)
for i, fpath in enumerate(fpaths):
    i_data_block = write_entry(FILES_BLOCK + i, fpath, i + 1 != len(fpaths), i_data_block)
