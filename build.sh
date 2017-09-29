set -e

ROOT=`pwd`
BIN=`pwd`/bin
SRC=`pwd`/src
BOOT=$SRC/boot
KERNEL=$SRC/kernel
LIBC=$SRC/libc
PROG=$SRC/prog
TOOL=$ROOT/tool

CFLAGS="-m32 -ffreestanding -nostdlib -nostdinc"

mkdir -p BIN

cd $BOOT
nasm mbr.asm -o $BIN/mbr.img -f bin
gcc boot.c -o $BIN/boot.o $CFLAGS -Wl,-Ttext=0x8000 -Wl,-ebootmain

cd $KERNEL
gcc *.c -masm=intel -o $BIN/kernel.img $CFLAGS -Wl,-Ttext=0xc0100000 -Wl,-eentry

cd $TOOL
./build-prog.py

cd $BIN
objcopy boot.o boot.img -j .text -O binary
dd if=/dev/zero of=eos.img bs=16M count=1 status=none
dd if=mbr.img of=eos.img conv=notrunc status=none
dd if=boot.img of=eos.img seek=1 conv=notrunc status=none
dd if=kernel.img of=eos.img seek=8 conv=notrunc status=none

cd $TOOL
./bundle.py
