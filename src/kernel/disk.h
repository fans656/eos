#ifndef DISK_H
#define DISK_H

#include "types.h"

#define BYTES_PER_SECTOR 512
#define BPS BYTES_PER_SECTOR

#define BYTES_PER_BLOCK 4096
#define BPB BYTES_PER_BLOCK

typedef struct {
    ulonglong n_sectors;
    uint bytes_per_sector;
} DiskInfo;

extern DiskInfo disk_info;

void init_disk();

void read_bytes(ulonglong i_byte, ulonglong n_bytes, void* buffer);
void write_bytes(ulonglong i_byte, ulonglong n_bytes, void* data);

void read_sector(uint i_sector, void* buffer);
void write_sector(uint i_sector, void* data);

void read_sectors(uint i_sector, uchar n_sectors, void* buffer);
void write_sectors(uint i_sector, uchar n_sectors, void* data);

#endif
