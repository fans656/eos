nasm -f bin -i src/bootloader/ -o bin/eos.img src/bootloader/bootloader.asm &&\
    gcc -m32 -ffreestanding -masm=intel -Wl,-Ttext=0x8000 -Wl,-ekernel_entry -nostdlib src/kernel/*.c -o bin/kernel.out &&\
    scripts/make_image_from_out.py &&\
    cat bin/kernel.img >> bin/eos.img
