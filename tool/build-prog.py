#!/usr/bin/env python
import os


os.chdir('../src/prog')
os.system('mkdir -p ../../files/bin')
for fname in os.listdir('.'):
    os.chdir(fname)
    ret = os.system('gcc *.c ../../libc/*.c '
              '-m32 -ffreestanding -nostdlib -nostdinc '
              '-masm=intel '
              '-I ../../libc '
              '-Wl,-e_start '
              '-o ../../../files/bin/{}'.format(fname))
    if ret:
        exit(1)
    os.chdir('..')
