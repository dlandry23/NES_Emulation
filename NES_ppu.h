/*
NES_ppu.h
*/
#include <stdint.h>

typedef struct PPU
{
    uint8_t a;
}PPU;

PPU ppu_init();
uint8_t ppu_read(PPU *ppu, uint16_t addr);
void ppu_write(PPU *ppu, uint16_t addr,uint8_t data);