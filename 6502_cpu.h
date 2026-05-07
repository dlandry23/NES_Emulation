/*
6502_cpu.h
struct CPU outline registers -> a couple of placeholder functions for reset, initialize, and such
*/
#include <stdint.h>

typedef struct CPU {
    uint8_t a, x, y;
    uint8_t s; //Stack Pointer
    uint8_t p; //Status Register
    uint16_t pc; // Program Counter (address essentially)
} CPU;

typedef struct {
    void (*operate)(CPU*, uint16_t);
    uint16_t (*addrmode)(CPU*,int);
    uint8_t cycles;
} Instruction;

typedef struct BUS {
    uint8_t ram[0x0800];
    
    uint8_t rom[0x8000];
    uint8_t *prg_rom;
    uint32_t prg_size;

    void *cart;

    struct PPU *ppu;
    struct APU *apu;

}BUS;

void cpu_step();
uint8_t bus_read (BUS *bus, uint16_t addr);
void bus_write (BUS *bus, uint16_t addr, uint8_t data);
//uint8_t cpu_read(uint16_t);
//void cpu_write(uint16_t,uint8_t);