set -e

ROOT=`pwd`
BIN=`pwd`/bin
SRC=`pwd`/src
BOOT=$SRC/boot
KERNEL=$SRC/kernel

CFLAGS="-m32 -ffreestanding -nostdlib"

mkdir -p BIN

cd $BOOT
nasm mbr.asm -o $BIN/mbr.img -f bin
gcc boot.c -o $BIN/boot.o $CFLAGS -Wl,-Ttext=0x8000 -Wl,-ebootmain

cd $KERNEL
gcc *.c -o $BIN/kernel.img $CFLAGS -Wl,-Ttext=0xc0100000 -Wl,-emain

cd $BIN
objcopy boot.o boot.img -j .text -O binary
dd if=/dev/zero of=eos.img count=2048
dd if=mbr.img of=eos.img conv=notrunc
dd if=boot.img of=eos.img seek=1 conv=notrunc
dd if=kernel.img of=eos.img seek=8 conv=notrunc
