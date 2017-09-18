#!/usr/bin/env python
with open('mbr.img', 'rb+') as f:
    f.seek(510)
    f.write('\x55\xaa')
