#!/usr/bin/env python
import os


os.chdir('../src/prog')
os.system('mkdir -p ../../files/bin')
for fname in os.listdir('.'):
    os.chdir(fname)
    ret = os.system('g++ *.cc ../../libc/*.cc '
              '-m32 -ffreestanding -fno-exceptions -fno-rtti -nostdlib -nostdinc -std=c++11 '
              '-masm=intel '
              '-I ../../libc -I ../../common '
              '-Wl,-e_start '
              '-o ../../../files/bin/{}'.format(fname))
    if ret:
        exit(1)
    os.chdir('..')
