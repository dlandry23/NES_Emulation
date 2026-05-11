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

typedef struct NESfile {
    char *filename;
    uint8_t header[0x10]; //16-byte header
    uint8_t *prg_rom;
    uint8_t *chr_rom;

    uint8_t prg_rom_size; // in 16KB units
    uint8_t chr_rom_size; // in 8KB units
    uint8_t mapper; // not using for now - just for future if I implement a wrapper outside of MMC3
} NESfile;

typedef struct MAPPER{
    uint8_t R[0x08];
    uint8_t bank_select;
} MAPPER;

typedef struct BUS {
    uint8_t ram[0x0800];
    
    /*uint8_t rom[0x8000];
    uint8_t *prg_rom;
    uint32_t prg_size;

    void *cart;*/
    uint8_t *prg_banks[0x04];
    uint8_t *chr_banks[0x06];
    struct MAPPER mapper;
    struct NESfile rom_file;
    struct PPU *ppu;
    struct APU *apu;

}BUS;

//NES file initialization
void NESfile_init(NESfile *rom_file);

void cpu_step();
uint8_t bus_read (BUS *bus, uint16_t addr);
void bus_write (BUS *bus, uint16_t addr, uint8_t data);
//uint8_t cpu_read(uint16_t);
//void cpu_write(uint16_t,uint8_);