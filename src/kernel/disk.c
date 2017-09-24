/*
http://wiki.osdev.org/ATA_PIO_Mode
http://wiki.osdev.org/PCI_IDE_Controller

- LBA28 is faster for LBA48, so it can be used when disk is less than 128GB
 */
#include "disk.h"
#include "asm.h"
#include "stdio.h"
#include "string.h"

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

uchar sector_buffer[BPS];

void wait_for(int port, uchar mask, uchar val) {
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
        uchar status = inb(ALT_STATUS_PORT);
        if (status & BSY) {
            continue;
        }
        if (status & ERR) {
            panic("Error in wait_until_ready\n");
        }
        if (status & DWF) {
            panic("Drive wite fault in wait_until_ready\n");
        }
        if ((status & DRQ) == DRQ) {
            return;
        }
    }
}

typedef struct {
        ushort _notcare;  // word 0

    ushort n_cylinders;  // word 1

        ushort _zero;  // word 2

    ushort n_heads;  // word 3
    ushort bytes_per_track;  // word 4
    ushort bytes_per_sector;  // word 5
    ushort sectors_per_track;  // word 6

        ushort _reserved[3];  // word 7-9

    char serial_number[20];  // word 10-19

        ushort __notcare[29];  // word 20-48

        ushort __reserved : 8;  // word 49

    ushort lba_supported : 1;  // word 49
    ushort dma_supported : 1;  // word 49

        ushort ___notcare[10];  // word 50-59
        uint n_total_sectors_lba28; // word 60, 61
        ushort ____notcare[38];  // word 62-99
        ulonglong n_total_sectors_lba48; // word 100-103
        ushort ___more[152];  // word 104-255
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
        *((ushort*)(&info) + i) = inw(DATA_PORT);
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

void read_bytes(ulonglong i_byte, ulonglong n_bytes, void* buffer) {
    uint i_sector = i_byte / BPS;
    uchar* p = buffer;
    if (i_byte % BPS != 0) {
        read_sector(i_sector++, sector_buffer);
        uint n_left = i_byte % BPS;
        uint n_read = BPS - n_left;
        memcpy(p, sector_buffer + n_left, n_read);
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
        memcpy(p, sector_buffer, n_bytes);
        p += n_bytes;
    }
}

void write_bytes(ulonglong i_byte, ulonglong n_bytes, uchar* data) {
    uint i_sector = i_byte / BPS;
    if (i_byte % BPS) {
        uint n_left = i_byte % BPS;
        uint n_write = BPS - n_left;
        read_sector(i_sector, sector_buffer);
        memcpy(sector_buffer + n_left, data, n_write);
        write_sector(i_sector++, sector_buffer);
        data += n_write;
        n_bytes -= n_write;
    }
    while (n_bytes > 0 && n_bytes % BPS == 0) {
        write_sector(i_sector++, data);
        data += BPS;
        n_bytes -= BPS;
    }
    if (n_bytes % BPS) {
        uint n_write = n_bytes % BPS;
        read_sector(i_sector, sector_buffer);
        memcpy(sector_buffer, data, n_write);
        write_sector(i_sector++, sector_buffer);
    }
}

void read_sector(uint i_sector, uchar* buffer) {
    read_sectors(i_sector, 1, buffer);
}

void write_sector(uint i_sector, uchar* data) {
    write_sectors(i_sector, 1, data);
}

void prepare_read_write(uint i_sector, uchar n_sectors) {
    outb(DRIVE_PORT, 0xe0 | ((i_sector >> 24) & 0x0f));
    outb(SECTOR_COUNT_PORT, n_sectors);
    outb(LBA_LOW_PORT, i_sector & 0xff);
    outb(LBA_MID_PORT, (i_sector >> 8) & 0xff);
    outb(LBA_HIGH_PORT, (i_sector >> 16) & 0xff);
}

void read_sectors(uint i_sector, uchar n_sectors, uchar* buffer) {
    asm("cli");

    prepare_read_write(i_sector, n_sectors);
    outb(COMMAND_PORT, READ_SECTORS_CMD);
    wait_until_ready();

    ushort* p = (ushort*)buffer;
    while (n_sectors--) {
        for (int i = 0; i < 256; ++i) {
            *p++ = inw(DATA_PORT);
        }
        wait_for(ALT_STATUS_PORT, BSY, 0);
    }

    asm("sti");
}

void write_sectors(uint i_sector, uchar n_sectors, uchar* data) {
    asm("cli");

    prepare_read_write(i_sector, n_sectors);
    outb(COMMAND_PORT, WRITE_SECTORS_CMD);
    wait_until_ready();

    ushort* p = (ushort*)data;
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
