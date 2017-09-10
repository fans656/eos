mkdir -p bin
touch bin/eos.img
nasm -f bin -i src/bootloader/ -o bin/mbr.img src/bootloader/bootloader.asm &&\
    gcc src/kernel/*.c -ffreestanding -m32 -masm=intel -nostdlib -static-libgcc -lgcc -Wl,-Ttext=0x8000 -Wl,-ekernel_entry -o bin/kernel.out &&\
    scripts/make_image_from_out.py &&
    scripts/file-bundler/bundle.py
