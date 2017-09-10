#! /usr/bin/env python
import os

from filesystem import Filesystem


os.chdir(os.path.dirname(os.path.abspath(__file__)))

fs = Filesystem()
fs.format()
fs.bundle_directory('../../files')

with open('disk.img', 'rb') as src, open('../../bin/eos.img', 'rb+') as dst:
    src.seek(0x100000)
    dst.seek(0x100000)
    while True:
        data = src.read(4096)
        if not data:
            break
        dst.write(data)
