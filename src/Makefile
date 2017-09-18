CC = gcc
LD = ld
NASM = nasm
OBJCOPY = objcopy
CFLAGS = -ffreestanding -masm=intel -m32
LDFLAGS = -m elf_i386

mbr.img: mbr0.s mbr1.c
	$(NASM) -f elf mbr0.s
	$(CC) $(CFLAGS) -c mbr1.c
	$(LD) $(LDFLAGS) -Ttext 0x7c00 -o mbr.o mbr0.o mbr1.o
	$(OBJCOPY) -j .text -S -O binary mbr.o mbr.img
	./sign-mbr.py

clean:
	rm *.o *.img
