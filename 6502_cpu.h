/*
6502_cpu.h
struct CPU outline registers -> a couple of placeholder functions for reset, initialize, and such
*/
#ifndef CPU_6502_H
#define CPU_6502_H

#include <stdint.h>



typedef struct CPU {
    uint8_t a, x, y;
    uint8_t s; //Stack Pointer
    uint8_t p; //Status Register
    uint16_t pc; // Program Counter (address essentially)
    
    uint8_t cycles;
} CPU;

typedef struct BUS {
    uint8_t ram[0x0800];
    uint8_t *prg_banks[0x04];
    uint8_t *chr_banks[0x06];
    struct MAPPER *mapper;
    struct NESfile *rom_file;
    struct PPU *ppu;
    struct APU *apu;

}BUS;

typedef struct Instruction {
    void (*operate)(CPU*, BUS*, uint16_t);
    uint16_t (*addrmode)(CPU*, BUS*, uint8_t*);
    uint8_t cycles;
} Instruction;



//NES file initialization


void cpu_step(CPU *cpu, BUS *bus);
uint8_t bus_read (BUS *bus, uint16_t addr);
void bus_write (BUS *bus, uint16_t addr, uint8_t data);
void init_banks (BUS *bus);
//uint8_t cpu_read(uint16_t);
//void cpu_write(uint16_t,uint8_);
#endif