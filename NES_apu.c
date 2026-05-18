/*
NES_apu.c
*/
#include <stdio.h>
#include "NES_apu.h"

APU apu_init()
{
    APU apu = 
    {
        .a=0x00
    };
    return apu;
}

uint8_t apu_read(APU *apu, uint16_t addr)
{
    //TODO - Write this thang
    printf("APU READ\n");
    return 0x00;
}

void apu_write(APU *ppu, uint16_t addr, uint8_t data)
{
    printf("APU WRITE\n");
    // TODO - this
}