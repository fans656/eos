nasm -f bin -o eos.img bootloader.asm
curl -XPOST --data-binary "@eos.img" host:6560?debug
gdb -x cmds.gdb
