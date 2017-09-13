#! /usr/bin/env python
import os


if os.getcwd().endswith('scripts'):
    os.chdir('..')
os.chdir('src/prog')

for fname in os.listdir('.'):
    if os.path.isdir(fname):
        cmd = ('gcc {name}/*.c ../libc/*.c '
               + '-m32 -masm=intel -nostdlib -ffreestanding -static-libgcc -lgcc '
               + '-I../libc '
               + '-Wl,-Ttext=0x4000000 -Wl,-emain '
               + '-o ../../files/bin/{name}').format(name=fname)
        os.system(cmd)
