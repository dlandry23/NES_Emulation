/*
NES_ppu.c
*/
#include <stdio.h>
#include "NES_ppu.h"

PPU ppu_init()
{
    PPU ppu = 
    {
        .a=0x00
    };
    return ppu;
}

uint8_t ppu_read(PPU *ppu, uint16_t addr)
{
    printf("NOT IMPLEMENTED YET");
    return 0x00;
    //TODO - Write this thang
}

void ppu_write(PPU *ppu, uint16_t addr,uint8_t data)
{
    printf("NOT IMPLEMENTED YET");
    // TODO - this
}