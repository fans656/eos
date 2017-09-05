#! /usr/bin/env python
import os

from filesystem import FileSystem
from conf import *


fs = FileSystem(IMAGE_FPATH)
fs.format()
#fs.ls()
fs.external_copy('girl2.jpg', 'girl2.jpg')
#fs.ls()
s = fs.open('girl2.jpg').read()
print len(s)
exit()

while True:
    try:
        cmd = raw_input('$ ')
    except EOFError:
        break
    if cmd == 'exit':
        break
    elif cmd == 'ls':
        fs.ls()
    elif cmd == 'pwd':
        print fs.pwd()
    elif cmd == 'fmt':
        fs.format()
    elif cmd == 'info':
        fs.show()
    elif cmd.startswith('bit'):
        fs.show_bitmap()
    elif cmd.startswith('ecp'):
        src, dst = cmd.split()[1:]
        fs.external_copy(src, dst)
fs.close()
