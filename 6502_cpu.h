/*
6502_cpu.h
struct CPU outline registers -> a couple of placeholder functions for reset, initialize, and such
*/
#ifndef CPU_6502_H
#define CPU_6502_H

#include <stdint.h>
#include "NES_file.h"
#include "NES_ppu.h"
#include "NES_apu.h"


typedef struct CPU {
    uint8_t a, x, y;
    uint8_t s; //Stack Pointer
    uint8_t p; //Status Register
    uint16_t pc; // Program Counter (address essentially)
    
    uint8_t cycles;
    uint8_t opcode;
    char *opcode_asm;
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

typedef struct MAPPER{
    uint8_t R[0x08];
    uint8_t bank_select;
} MAPPER;

typedef struct Instruction {
    void (*operate)(CPU*, BUS*, uint16_t);
    uint16_t (*addrmode)(CPU*, BUS*, uint8_t*);
    uint8_t cycles;
} Instruction;

typedef struct {
    char *name;
    uint8_t     bytes;
} OpcodeInfo;

//NES file initialization

CPU cpu_init();
MAPPER mapper_init();
void cpu_reset(CPU *cpu, BUS *bus);
BUS bus_init(NESfile *rom_file,PPU *ppu, APU *apu, MAPPER *mapper);
void cpu_step(CPU *cpu, BUS *bus);
uint8_t bus_read (BUS *bus, uint16_t addr);
void bus_write (BUS *bus, uint16_t addr, uint8_t data);
void init_banks (BUS *bus);
void bank_switch(BUS *bus,uint8_t data);
void init_table();
void disassemble(uint8_t opcode, uint16_t pc, BUS *bus, char *out, size_t out_size);
//uint8_t cpu_read(uint16_t);
//void cpu_write(uint16_t,uint8_);
#endif