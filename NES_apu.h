/*
NES_apu.h
*/

#include <stdint.h>
typedef struct APU
{
    uint8_t a;
}APU;

uint8_t apu_read(APU *apu, uint16_t addr);