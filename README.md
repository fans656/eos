# eos - a homemade OS

## Milestones

- Enter protected mode
- Print message using C
- Keyboard input by polling

## Future

- Keyboard input by interrupt
- File system
- ELF executable
- Multi-processing
- Graphical user interface
- Networking
- Mouse
- Audio

## Develop Environment

- Windows 10 laptop
- Writing code in an Ubuntu VM
- Compile the kernel image (including bootloader MBR) on VM
- Send it to host OS where a HTTP server is running to receive POSTed file
- Host batch script run to start `qemu` using the POSTed kernel image
