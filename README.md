# eos - a homemade OS

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

## Future

- Console
- Virtual memory
- Multi-processing
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

## Develop Environment

- Windows 10 laptop
- Writing code in an Ubuntu VM
- Compile the kernel image (including bootloader MBR) on VM
- Send it to host OS where a HTTP server is running to receive POSTed file
- Host batch script run to start `qemu` using the POSTed kernel image

## References

[osdev](http://wiki.osdev.org/Main_Page)

[xv6](https://pdos.csail.mit.edu/6.828/2012/xv6.html)

[ORANGE’S：一个操作系统的实现](https://book.douban.com/subject/3735649/)

<div><p><a href="http://wiki.osdev.org/Main_Page">ATA Interface Reference Manual - Seagate</a></p></div>

[Intel® 64 and IA-32 Architectures Software Developer Manuals](https://software.intel.com/en-us/articles/intel-sdm)

[Modern Operating Systems - 4th Edition](https://github.com/yuanhui-yang/Modern-Operating-Systems/blob/master/Modern%20Operating%20Systems%20-%204th%20Edition.pdf)
