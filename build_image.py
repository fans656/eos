#!/usr/bin/env python
import os

with open('eos.img', 'rw+') as f:
    f.seek(512, os.SEEK_SET)
    f.write('\xeb\xfe' * 256)
