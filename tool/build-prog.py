#!/usr/bin/env python
import os


#makefile = '''
#PROG=..
#SRC=$(PROG)/..
#ROOT=$(SRC)/..
#FILES=$(ROOT)/files
#BIN=$(FILES)/bin
#LIB=$(SRC)/lib
#
#$(BIN)/{name}: $(LIB)/*.cc $(LIB)/gui/*.cc *.cc
#    g++ *.cc $(LIB)/*.cc ../../lib/gui/*.cc
#    -m32 -ffreestanding -fno-exceptions -fno-rtti -nostdlib -nostdinc -std=c++11
#    -masm=intel
#    -I ../../lib -I ../../common
#    -Wl,-e_start
#    -o ../../../files/bin/{}
#'''.format(fname)


os.chdir('src/prog')
os.system('mkdir -p ../files/bin')
for fname in os.listdir('.'):
    os.chdir(fname)
    ret = os.system('g++ *.cc ../../lib/*.cc ../../lib/gui/*.cc '
              '-m32 -ffreestanding -fno-exceptions -fno-rtti -nostdlib -nostdinc -std=c++11 '
              '-masm=intel '
              '-I ../../lib -I ../../common '
              '-Wl,-e_start '
              '-o ../../../files/bin/{}'.format(fname))
    if ret:
        exit(1)
    os.chdir('..')
