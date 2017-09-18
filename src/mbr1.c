void bootmain() {
    asm("mov word ptr [0xb8000], 0x0f41");
}
