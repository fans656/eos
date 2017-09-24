#ifndef DISK_H
#define DISK_H

#include "types.h"

#define BYTES_PER_SECTOR 512
#define BPS BYTES_PER_SECTOR

#define BYTES_PER_BLOCK 4096
#define BPP BYTES_PER_BLOCK

typedef struct {
    ulonglong n_sectors;
    uint bytes_per_sector;
    uint n_blocks;
} DiskMeta;

extern DiskMeta disk_meta;

void init_disk();

void read_bytes(ulonglong i_byte, ulonglong n_bytes, void* buffer);
void write_bytes(ulonglong i_byte, ulonglong n_bytes, uchar* data);

void read_sector(uint i_sector, uchar* buffer);
void write_sector(uint i_sector, uchar* data);

void read_sectors(uint i_sector, uchar n_sectors, uchar* buffer);
void write_sectors(uint i_sector, uchar n_sectors, uchar* data);

#endif
