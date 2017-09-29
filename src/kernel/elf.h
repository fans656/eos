typedef struct {
    uint magic;
    uchar ignored[12];
    ushort type;
    ushort machine;
    uint version;
    uint entry;
    uint phoff;
    uint shoff;
    uint flags;
    ushort ehsize;
    ushort phentsize;
    ushort phnum;
    ushort shentsize;
    ushort shnum;
    ushort shstrndx;
} ELFHeader;

typedef struct {
    uint type;
    uint offset;
    uint vaddr;
    uint paddr;
    uint filesz;
    uint memsz;
    uint flags;
    uint align;
} ProgramHeader;
