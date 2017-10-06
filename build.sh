set -e  # stop on error

CC=g++

CFLAGS="-m32 -ffreestanding -fno-exceptions -fno-rtti -fno-use-cxa-atexit \
    -nostdlib -nostdinc -masm=intel -static-libgcc -lgcc -std=c++14"
DDFLAGS="conv=notrunc status=none"

ROOT=`pwd`

BIN=$ROOT/bin
SRC=$ROOT/src
TOOL=$ROOT/tool

KERNEL=$SRC/kernel
BOOT=$KERNEL/boot

LIBC=$SRC/libc
PROG=$SRC/prog

mkdir -p $BIN

cd $BOOT
nasm mbr.asm -o $BIN/mbr.img -f bin
$CC $CFLAGS boot.c -o $BIN/boot.o -Wl,-Ttext=0x7e00 -Wl,-ebootmain

cd $KERNEL
nasm -f elf isr.asm -o $BIN/isr.o
$CC $CFLAGS *.cc $BIN/isr.o -o $BIN/kernel.img -Wl,-Ttext=0xc0100000 -Wl,-eentry

cd $TOOL
./build-prog.py  # compile user programs

cd $BIN
objcopy boot.o boot.img -j .text -O binary  # copy boot.o .text so mbr can jmp to it
dd $DDFLAGS if=/dev/zero of=eos.img bs=16M count=1  # disk size 16MB
dd $DDFLAGS if=mbr.img of=eos.img  # mbr at sector [0..1)
dd $DDFLAGS if=boot.img of=eos.img seek=1  # 2-stage bootloader at sector [1..8)
dd $DDFLAGS if=kernel.img of=eos.img seek=8  # kernel at sector [4K..1MB)

cd $TOOL
./bundle.py
