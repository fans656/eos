/*
http://wiki.osdev.org/ATA_PIO_Mode
http://wiki.osdev.org/PCI_IDE_Controller

- LBA28 is faster for LBA48, so it can be used when disk is less than 128GB
 */
#include <stdint.h>
#include <stdbool.h>
#include "disk.h"
#include "io.h"
#include "util.h"

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

#define READ_SECTORS_CMD 0x20
#define WRITE_SECTORS_CMD 0x30
#define READ_SECTORS_EXT_CMD 0x24
#define WRITE_SECTORS_EXT_CMD 0x34
#define CACHE_FLUSH_CMD 0xe7
#define IDENTITY_CMD 0xec

#define BSY 0x80
#define DRDY 0x40
#define DWF 0x20
#define DRQ 0x08
#define ERR 0x01

#define low_byte(x) ((x) & 0xff)
#define high_byte(x) (((x) >> 8) & 0xff)

DiskMeta disk_meta;

uint8_t sector_buffer[BPS];

void wait_for(int port, uint8_t mask, uint8_t val) {
    while ((inb(port) & mask) != val) {
        ;
    }
}

void _400ns_delay() {
    inb(ALT_STATUS_PORT);
    inb(ALT_STATUS_PORT);
    inb(ALT_STATUS_PORT);
    inb(ALT_STATUS_PORT);
}

void wait_until_ready() {
    _400ns_delay();
    while (true) {
        uint8_t status = inb(ALT_STATUS_PORT);
        if (status & BSY) {
            continue;
        }
        if (status & ERR) {
            printf("Error in wait_until_ready\n");
            hlt();
        }
        if (status & DWF) {
            printf("Drive wite fault in wait_until_ready\n");
            hlt();
        }
        if ((status & DRQ) == DRQ) {
            return;
        }
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

        uint16_t ___notcare[10];  // word 50-59
        uint32_t n_total_sectors_lba28; // word 60, 61
        uint16_t ____notcare[38];  // word 62-99
        uint64_t n_total_sectors_lba48; // word 100-103
        uint16_t ___more[152];  // word 104-255
} IdentifyInfo;

void _100ns_delay() {
    inb(STATUS_PORT);
}

void select_master_drive() {
    outb(DRIVE_PORT, 0xe0);
    _400ns_delay();
}

void clear_address_ports() {
    outb(SECTOR_COUNT_PORT, 0);
    outb(LBA_LOW_PORT, 0);
    outb(LBA_MID_PORT, 0);
    outb(LBA_HIGH_PORT, 0);
}

void do_identify(DiskMeta* meta) {
    outb(COMMAND_PORT, IDENTITY_CMD);
    wait_until_ready();

    IdentifyInfo info;
    for (int i = 0; i < 256; ++i) {
        *((uint16_t*)(&info) + i) = inw(DATA_PORT);
    }
    wait_for(ALT_STATUS_PORT, BSY, 0);
    meta->n_sectors = info.n_total_sectors_lba48;
    meta->bytes_per_sector = info.bytes_per_sector;
}

void init_disk() {
    asm("cli");
    select_master_drive();
    clear_address_ports();
    do_identify(&disk_meta);
    asm("sti");
}

void read_bytes(uint64_t i_byte, uint64_t n_bytes, void* buffer) {
    uint32_t i_sector = i_byte / BPS;
    uint8_t* p = buffer;
    if (i_byte % BPS != 0) {
        read_sector(i_sector++, sector_buffer);
        uint32_t n_left = i_byte % BPS;
        uint32_t n_read = BPS - n_left;
        memcpy(sector_buffer + n_left, p, n_read);
        p += n_read;
        n_bytes -= n_read;
    }
    while (n_bytes >= BPS) {
        read_sector(i_sector++, p);
        n_bytes -= BPS;
        p += BPS;
    }
    if (n_bytes) {
        read_sector(i_sector++, sector_buffer);
        memcpy(sector_buffer, p, n_bytes);
        p += n_bytes;
    }
}

void write_bytes(uint64_t i_byte, uint64_t n_bytes, uint8_t* data) {
}

void read_sector(uint32_t i_sector, uint8_t* buffer) {
    read_sectors(i_sector, 1, buffer);
}

void write_sector(uint32_t i_sector, uint8_t* data) {
    write_sectors(i_sector, 1, data);
}

void prepare_read_write(uint32_t i_sector, uint8_t n_sectors) {
    outb(DRIVE_PORT, 0xe0 | ((i_sector >> 24) & 0x0f));
    outb(SECTOR_COUNT_PORT, n_sectors);
    outb(LBA_LOW_PORT, i_sector & 0xff);
    outb(LBA_MID_PORT, (i_sector >> 8) & 0xff);
    outb(LBA_HIGH_PORT, (i_sector >> 16) & 0xff);
}

void read_sectors(uint32_t i_sector, uint8_t n_sectors, uint8_t* buffer) {
    asm("cli");

    prepare_read_write(i_sector, n_sectors);
    outb(COMMAND_PORT, READ_SECTORS_CMD);
    wait_until_ready();

    uint16_t* p = (uint16_t*)buffer;
    while (n_sectors--) {
        for (int i = 0; i < 256; ++i) {
            *p++ = inw(DATA_PORT);
        }
        wait_for(ALT_STATUS_PORT, BSY, 0);
    }

    asm("sti");
}

void write_sectors(uint32_t i_sector, uint8_t n_sectors, uint8_t* data) {
    asm("cli");

    prepare_read_write(i_sector, n_sectors);
    outb(COMMAND_PORT, WRITE_SECTORS_CMD);
    wait_until_ready();

    uint16_t* p = (uint16_t*)data;
    while (n_sectors--) {
        for (int i = 0; i < 256; ++i) {
            outw(DATA_PORT, *p++);
        }
        wait_for(ALT_STATUS_PORT, BSY, 0);
        outb(COMMAND_PORT, CACHE_FLUSH_CMD);
        wait_for(ALT_STATUS_PORT, BSY, 0);
    }

    asm("sti");
}
