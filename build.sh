mkdir -p bin
nasm -f bin -i src/bootloader/ -o bin/mbr.img src/bootloader/bootloader.asm &&\
    gcc src/bootloader/*.c -ffreestanding -m32 -masm=intel -nostdlib -static-libgcc -lgcc -Wl,-Ttext=0x8000 -Wl,-ekernel_entry -o bin/kernel.out &&\
    scripts/make_image_from_out.py &&
    mkdir -p files/kernel
    gcc src/kernel/*.c -ffreestanding -m32 -masm=intel -nostdlib -static-libgcc -lgcc -Wl,-Ttext=0xc0000000 -Wl,-ekernel_entry -o files/kernel/kernel.img &&\
    scripts/compile_progs.py &&
    scripts/file-bundler/bundle.py
