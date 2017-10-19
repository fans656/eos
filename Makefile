DD=dd
CC=g++

CFLAGS=-m32 -ffreestanding -fno-exceptions -fno-rtti -fno-use-cxa-atexit \
    -nostdlib -nostdinc -masm=intel -static-libgcc -lgcc -std=c++14
DDFLAGS=conv=notrunc status=none

BIN=bin
TOOL=tool
SRC=src
KERNEL=$(SRC)/kernel
BOOT=$(KERNEL)/boot
LIB=$(SRC)/lib
PROG=$(SRC)/prog
USER_BIN=files/bin

LIB_INC=-I../../$(LIB)/../common -I../../$(LIB)  -I../../$(LIB)/libc
USER_INC=-I$(LIB) -I$(SRC)/common -I$(SRC)/lib/libc

run: $(BIN)/eos.img
	@curl -XPOST --data-binary "@bin/eos.img" host:6560

$(BIN)/eos.img: $(BIN)/mbr.img $(BIN)/boot.img $(BIN)/kernel.img $(TOOL)/bundle.py $(USER_BIN)/*
	@echo "=============================================== Making image file"
	@mkdir -p $(BIN)
	$(DD) $(DDFLAGS) if=/dev/zero of=$(BIN)/eos.img bs=16M count=1  # disk size 16MB
	$(DD) $(DDFLAGS) if=$(BIN)/mbr.img of=$(BIN)/eos.img  # mbr at sector [0..1)
	$(DD) $(DDFLAGS) if=$(BIN)/boot.img of=$(BIN)/eos.img seek=1  # 2-stage bootloader at sector [1..8)
	$(DD) $(DDFLAGS) if=$(BIN)/kernel.img of=$(BIN)/eos.img seek=8  # kernel at sector [4K..1MB)
	$(TOOL)/bundle.py

$(BIN)/mbr.img: $(BOOT)/mbr.asm
	@echo "=============================================== Compiling MBR"
	@mkdir -p $(BIN)
	nasm $(BOOT)/mbr.asm -o $(BIN)/mbr.img -f bin || exit

$(BIN)/boot.img: $(BOOT)/boot.c
	@echo "=============================================== Compiling bootloader"
	@mkdir -p $(BIN)
	$(CC) $(CFLAGS) $(BOOT)/boot.c -o $(BIN)/boot.o -Wl,-Ttext=0x7e00 -Wl,-ebootmain || exit
	objcopy $(BIN)/boot.o $(BIN)/boot.img -j .text -O binary  # copy boot.o .text so mbr can jmp to it

$(BIN)/kernel.img: $(KERNEL)/*.cc $(KERNEL)/*.h $(KERNEL)/isr.asm
	@echo "=============================================== Compiling kernel"
	@mkdir -p $(BIN)
	nasm -f elf $(KERNEL)/isr.asm -o $(BIN)/isr.o
	$(CC) $(CFLAGS) -I$(SRC)/common $(KERNEL)/*.cc $(BIN)/isr.o -o $(BIN)/kernel.img -Wl,-Ttext=0xc0100000 -Wl,-eentry || exit

$(USER_BIN)/*: $(PROG)/* $(BIN)/lib/*.o
	@echo "=============================================== Compiling user programs"
	@mkdir -p $(USER_BIN)
	for dirname in $(PROG)/*; do $(CC) $$dirname/*.cc $(BIN)/lib/*.o \
		$(CFLAGS) $(USER_INC) \
		-o $(USER_BIN)/`basename $$dirname` -Wl,-e_start || exit; done

$(BIN)/lib/*.o: $(LIB)/*.cc $(LIB)/gui/*.cc
	@echo "=============================================== Compiling library"
	@mkdir -p $(BIN)/lib
	cd $(BIN)/lib; $(CC) ../../$(LIB)/*.cc ../../$(LIB)/libc/*.cc ../../$(LIB)/gui/*.cc -c $(CFLAGS) $(LIB_INC) || exit; cd -

clean:
	rm -rf $(BIN)/*
	rm -rf $(USER_BIN)/*

loc:
	@find src -type f | xargs wc -l | sort
