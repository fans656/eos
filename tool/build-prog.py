#!/usr/bin/env python
import os


os.chdir('../src/prog')
os.system('mkdir -p ../../files')
for fname in os.listdir('.'):
    os.chdir(fname)
    ret = os.system('gcc *.c ../../libc/*.c '
              '-m32 -ffreestanding -nostdlib -nostdinc '
              '-masm=intel '
              '-I ../../libc '
              '-Wl,-emain '
              '-o ../../../files/{}'.format(fname))
    if ret:
        exit(1)
    os.chdir('..')
