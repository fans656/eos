gcc -m32 -ffreestanding -Wl,-Ttext=0x8000 -Wl,-ekernel_entry -nostdlib src/kernel/*.c -o bin/kernel.out &&\
    scripts/make_image_from_out.py &&\
    nasm -f bin -i src/bootloader/ -o bin/eos.img src/bootloader/bootloader.asm &&\
    cat bin/kernel.img >> bin/eos.img
