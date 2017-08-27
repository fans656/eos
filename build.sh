gcc -c -m32 -ffreestanding kernel.c &&\
    ld -melf_i386 -e kernel_entry -Ttext=0x8000 -Tdata=0x9000 kernel.o -o kernel.out &&\
    objcopy -O binary -j .text kernel.out 0.section &&\
    objcopy -O binary -j .rodata kernel.out 1.section &&\
    objcopy -O binary -j .eh_frame kernel.out 2.section &&\
    objcopy -O binary -j .data kernel.out 3.section &&\
    cat 0.section 1.section 2.section > kernel.img
    dd if=/dev/null of=kernel.img bs=1 count=1 seek=4096 >& /dev/null
    cat 3.section >> kernel.img
    nasm -f bin -o eos.img bootloader.asm
    cat kernel.img >> eos.img
