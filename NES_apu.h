/*
NES_apu.h
*/

#include <stdint.h>
typedef struct APU
{
    uint8_t a;
}APU;

APU apu_init();
uint8_t apu_read(APU *apu, uint16_t addr);
void apu_write(APU *ppu, uint16_t addr, uint8_t data);