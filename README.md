# eos - fans656's OS attempt

## Milestones

- Enter protected mode
- Print message using C
- Keyboard input by polling
- Keyboard input by interrupt
- Snake
- printf
- 320x240 256-color VGA graphics
- 1024x768 24-bit true color VESA graphics
- Simple malloc without free
- Simple read-only file system
- System call (int 0x80)
- ELF executable
- Paging
- malloc/free

## Future

- Multi-processing
- Console
- GUI
  - Mouse
  - Windows
  - Font
  - JPG/PNG/..
  - Video
  - Audio
- Networking
- UTF-8
- POSIX
- Swap

## Develop Environment

- Windows 10 laptop
- Writing code in an Ubuntu VM
- Compile the kernel image (including bootloader MBR) on VM
- Send it to host OS where a HTTP server is running to receive POSTed file
- Host batch script run to start `qemu` using the POSTed kernel image

## Details

### Compilation

Use `build.sh` to generate the disk image `eos.img` at `bin/`  
(need `nasm` and `gcc`)

- `src/kernel/boot/mbr.asm` => `bin/mbr.img`
- `src/kernel/boot/boot.c` => `bin/boot.o` => `bin/boot.img`
- `src/kernel/*.c` => `bin/kernel.img`
- `bin/mbr.img` + `bin/boot.img` + `bin/kernel.img` => first 1MB of disk image

## References

### General

[osdev wiki](http://wiki.osdev.org/Main_Page)

[xv6 - MIT 6.828 Educational OS](https://pdos.csail.mit.edu/6.828/2012/xv6.html)

[ORANGE’S：一个操作系统的实现](https://book.douban.com/subject/3735649/)

[Intel® 64 and IA-32 Architectures Software Developer Manuals](https://software.intel.com/en-us/articles/intel-sdm)

[Modern Operating Systems - 4th Edition](https://github.com/yuanhui-yang/Modern-Operating-Systems/blob/master/Modern%20Operating%20Systems%20-%204th%20Edition.pdf)

### Topics

[ATA Interface Reference Manual - Seagate](http://bit.ly/2ynS9BO)

[A Malloc Tutorial](http://www.inf.udec.cl/~leo/Malloc_tutorial.pdf)
