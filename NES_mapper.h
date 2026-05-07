/*
NES_mapper.h
*/
#include <stdint.h>
typedef struct ROM
{
    uint8_t a;
}ROM;
void rom_load(char*);
//uint8_t rom_read(uint8_t *rom, uint16_t addr);
void mapper_write(uint8_t *rom, uint16_t addr,uint8_t data);