/*
NES_ppu.h
*/
#include <stdint.h>
#include "6502_cpu.h"

typedef struct PPU
{
    uint8_t a;
}PPU;

uint8_t ppu_read(PPU *ppu, uint16_t addr);