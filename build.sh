#gcc -c -m32 -ffreestanding kernel.c &&\
#    ld -melf_i386 -e kernel_entry -Ttext=0x8000 -Tdata=0x9000 kernel.o -o kernel.out &&\
#    objcopy -O binary -j .text kernel.out 0.section &&\
#    objcopy -O binary -j .rodata kernel.out 1.section &&\
#    objcopy -O binary -j .data kernel.out 2.section &&\
#    #cat /dev/null
#    # padding .text to align 16 bytes
#    ./padding_text_section.py &&\
#    # merge .text and .rodata
#    cat 0.section 1.section > kernel.img
#    # padding .text and .rodata up to 4K cause .data begin at 0x9000 - 0x8000 = 0x1000 = 4K
#    dd if=/dev/null of=kernel.img bs=1 count=1 seek=4096 >& /dev/null &&
#    # merge .data
#    cat 2.section >> kernel.img
#    nasm -f bin -o eos.img bootloader.asm
#    cat kernel.img >> eos.img

gcc -m32 -ffreestanding -Wl,-Ttext=0x8000 -Wl,-ekernel_entry -nostdlib kernel.c -o kernel.out &&\
    ./make_image_from_out.py &&\
    nasm -f bin -o eos.img bootloader.asm &&\
    cat kernel.img >> eos.img
    #cat /dev/null
