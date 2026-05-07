/*
6502_cpu.h
struct CPU outline registers -> a couple of placeholder functions for reset, initialize, and such
*/
#include <stdint.h>

typedef struct CPU {
    uint8_t a, x, y;
    uint8_t s; //Stack Pointer
    uint8_t p; //Status Register
    uint16_t pc;
} CPU;

typedef struct {
    void (*operate)(CPU*, uint16_t);
    uint16_t (*addrmode)(CPU*,int);
    uint8_t cycles;
} Instruction;

void cpu_step();