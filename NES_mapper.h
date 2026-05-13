/*
NES_mapper.h
*/
#include <stdint.h>
#include "6502_cpu.h"

typedef struct MAPPER{
    uint8_t R[0x08];
    uint8_t bank_select;
} MAPPER;

//uint8_t rom_read(uint8_t *rom, uint16_t addr);
void bank_switch(BUS *bus,uint8_t data);
//void mapper_write(uint8_t *prg_banks,uint8_t *chr_banks, uint16_t addr,uint8_t data);