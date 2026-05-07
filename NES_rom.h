/*
NES_rom.h
*/

typedef struct ROM
{
    uint8_t a;
}ROM;
void rom_load(char*);
uint8_t rom_read(ROM *rom, uint16_t addr);
void rom_write(ROM *rom, uint16_t addr,uint8_t data);