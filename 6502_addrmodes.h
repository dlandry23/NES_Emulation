/*
6502_addrmodes.h
TODO

*/

#include <stdint.h>
#include "6502_cpu.h"

uint16_t addr_imm(CPU *cpu,int);
uint16_t addr_zp(CPU *cpu,int);
uint16_t addr_zpx(CPU *cpu,int);
uint16_t addr_abs(CPU *cpu,int);
uint16_t addr_absx(CPU *cpu,int);
uint16_t addr_absy(CPU *cpu,int);
uint16_t addr_indirx(CPU *cpu,int);
uint16_t addr_indiry(CPU *cpu,int);
// fill in the rest