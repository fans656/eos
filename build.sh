set -e  # stop on error

ROOT=`pwd`

BIN=$ROOT/bin
SRC=$ROOT/src
TOOL=$ROOT/tool

KERNEL=$SRC/kernel
BOOT=$KERNEL/boot

LIBC=$SRC/libc
PROG=$SRC/prog

CFLAGS="-m32 -ffreestanding -nostdlib -nostdinc -masm=intel"
DDFLAGS="conv=notrunc status=none"

mkdir -p $BIN

cd $BOOT
nasm mbr.asm -o $BIN/mbr.img -f bin
gcc $CFLAGS boot.c -o $BIN/boot.o -Wl,-Ttext=0x7e00 -Wl,-ebootmain

cd $KERNEL
gcc $CFLAGS *.c -o $BIN/kernel.img -Wl,-Ttext=0xc0100000 -Wl,-eentry

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
