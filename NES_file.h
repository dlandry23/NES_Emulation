/*
NES_file.h
*/

#include <stdint.h>
typedef struct NESfile {
    char *filename;
    uint8_t header[0x10]; //16-byte header
    uint8_t *prg_rom;
    uint8_t *chr_rom;

    uint8_t prg_rom_size; // in 16KB units
    uint8_t chr_rom_size; // in 8KB units
    uint8_t mapper; // not using for now - just for future if I implement a wrapper outside of MMC3
} NESfile;

NESfile NESfile_init(char *filename);