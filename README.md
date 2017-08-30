# eos - a homemade OS

## Develop Environment

- Windows 10 laptop
- Writing code in an Ubuntu VM
- Compile the kernel image (including bootloader MBR) on VM
- Send it to host OS where a HTTP server is running to receive POSTed file
- Host batch script run to start `qemu` using the POSTed kernel image

## Milestones

- Enter protected mode
- Print message using C

## TODO

- Keyboard input
- File system
- ELF executable
- Multi-processing
- Graphical user interface
- Networking
- Mouse
- Audio
