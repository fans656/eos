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

## Future

- File system
- Memory Paging
- malloc/free
- ELF executable
- Multi-processing
- GUI
- Networking
- Mouse
- Audio

## Develop Environment

- Windows 10 laptop
- Writing code in an Ubuntu VM
- Compile the kernel image (including bootloader MBR) on VM
- Send it to host OS where a HTTP server is running to receive POSTed file
- Host batch script run to start `qemu` using the POSTed kernel image
