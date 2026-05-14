/*
6502_addrmodes.h
TODO

*/

#include <stdint.h>
#include "6502_cpu.h"

uint16_t addr_imm(CPU *cpu, int *page_cross);
uint16_t addr_zp(CPU *cpu, BUS *bus, int *page_cross);
uint16_t addr_zpx(CPU *cpu, BUS *bus, int *page_cross);
uint16_t addr_abs(CPU *cpu, BUS *bus, int *page_cross);
uint16_t addr_absx(CPU *cpu, BUS *bus, int *page_cross);
uint16_t addr_absy(CPU *cpu, BUS *bus, int *page_cross);
uint16_t addr_indirx(CPU *cpu, BUS *bus, int *page_cross);
uint16_t addr_indiry(CPU *cpu, BUS *bus, int *page_cross);
uint16_t addr_rel(CPU *cpu, BUS *bus, int *page_cross);
// fill in the rest