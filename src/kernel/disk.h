#ifndef DISK_H
#define DISK_H

#include <stdint.h>

typedef struct {
    uint64_t n_sectors;
    uint16_t bytes_per_sector;
} DiskMeta;

extern DiskMeta disk_meta;

void init_disk();

void read_sector(uint32_t i_sector, uint8_t* buffer);
void write_sector(uint32_t i_sector, uint8_t* data);

void read_sectors(uint32_t i_sector, uint8_t n_sectors, uint8_t* buffer);
void write_sectors(uint32_t i_sector, uint8_t n_sectors, uint8_t* data);

#endif
