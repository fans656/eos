#ifndef DISK_H
#define DISK_H

#include <stdint.h>

#define BYTES_PER_SECTOR 512
#define BPS BYTES_PER_SECTOR

#define BYTES_PER_BLOCK 4096
#define BPP BYTES_PER_BLOCK

typedef struct {
    uint64_t n_sectors;
    uint32_t bytes_per_sector;
    uint32_t n_blocks;
} DiskMeta;

extern DiskMeta disk_meta;

void init_disk();

void read_bytes(uint64_t i_byte, uint64_t n_bytes, void* buffer);

void read_sector(uint32_t i_sector, uint8_t* buffer);
void write_sector(uint32_t i_sector, uint8_t* data);

void read_sectors(uint32_t i_sector, uint8_t n_sectors, uint8_t* buffer);
void write_sectors(uint32_t i_sector, uint8_t n_sectors, uint8_t* data);

#endif
