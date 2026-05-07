/*
6502_opcodes.h
TODO
Add list of opcodes in format of void command(CPU *cpu,uint16_t addr) <- this is how it will be implemented in opcodes.c so this one can have
void command(CPU*, uint16_t) --- uint16_t because program counter is 16bits/2bytes

*/
#include <stdint.h>
#include "6502_cpu.h"//Access to CPU typedef stuct{}

//Arithmetic
void adc(CPU *cpu, uint16_t addr); // Add with Carry
void sbc(CPU *cpu, uint16_t addr); // Subtract with Borrow
// CONTINUE ON DOWN FOR ALL (organize by function type to make it easier to read)