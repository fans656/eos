#include "io.h"
#include "interrupt.h"
#include "snake.h"
#include "util.h"
#include "draw.h"
#include "image.h"

void main();

void kernel_entry() {
    setup_idt();
    clear_screen();
    asm("cli");
    main();
    asm("sti");
    hlt();
}

#define DATA_PORT 0x1f0
#define ERROR_PORT 0x1f1
#define SECTOR_COUNT_PORT 0x1f2
#define LBA_LOW_PORT 0x1f3
#define LBA_MID_PORT 0x1f4
#define LBA_HIGH_PORT 0x1f5
#define DRIVE_PORT 0x1f6
#define STATUS_PORT 0x1f7
#define COMMAND_PORT 0x1f7
#define ALT_STATUS_PORT 0x3f6

#define MASTER_DRIVE 0xa0

#define IDENTITY_CMD 0xec

#define BSY 0x80
#define DRQ 0x08

void wait_for(int port, uint8_t mask, uint8_t val) {
    while (inb(port) & mask != val) {
        ;
    }
}

typedef struct {
        uint16_t _notcare;  // word 0

    uint16_t n_cylinders;  // word 1

        uint16_t _zero;  // word 2

    uint16_t n_heads;  // word 3
    uint16_t bytes_per_track;  // word 4
    uint16_t bytes_per_sector;  // word 5
    uint16_t sectors_per_track;  // word 6

        uint16_t _reserved[3];  // word 7-9

    char serial_number[20];  // word 10-19

        uint16_t __notcare[29];  // word 20-48

        uint16_t __reserved : 8;  // word 49

    uint16_t lba_supported : 1;  // word 49
    uint16_t dma_supported : 1;  // word 49

        uint16_t _more[10];  // word 50-59
        uint32_t n_total_sectors_lba28; // word 60, 61
        uint16_t __more[38];  // word 62-99
        uint64_t n_total_sectors_lba48; // word 100-103
        uint16_t ___more[152];  // word 104-255
} IdentifyInfo;

void main() {
    outb(DRIVE_PORT, MASTER_DRIVE);
    inb(STATUS_PORT);
    inb(STATUS_PORT);
    inb(STATUS_PORT);
    inb(STATUS_PORT);

    outb(SECTOR_COUNT_PORT, 0);
    outb(LBA_LOW_PORT, 0);
    outb(LBA_MID_PORT, 0);
    outb(LBA_HIGH_PORT, 0);

    outb(COMMAND_PORT, IDENTITY_CMD);
    
    uint8_t status = inb(STATUS_PORT);
    if (status == 0) {
        printf("Drive does not exists\n");
        return;
    }
    wait_for(STATUS_PORT, BSY, 0);
    if (inb(LBA_MID_PORT) || inb(LBA_HIGH_PORT)) {
        printf("Not ATA disk!\n");
        return;
    }
    wait_for(STATUS_PORT, DRQ, DRQ);
    
    IdentifyInfo info;
    for (int i = 0; i < 256; ++i) {
        *((uint16_t*)(&info) + i) = inw(DATA_PORT);
    }
    printf("LBA supported: %d\n", info.lba_supported);
    printf("DMA supported: %d\n", info.dma_supported);
    printf("Total sectors: %d\n", info.n_total_sectors_lba28);
    printf("Total sectors: %lld\n", info.n_total_sectors_lba48);
    printf("Bytes per sector: %d\n", info.bytes_per_sector);
    printf("Total bytes: %lld\n", info.bytes_per_sector * info.n_total_sectors_lba48);
}
