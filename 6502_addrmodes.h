/*
6502_addrmodes.h
TODO

*/

#include <stdint.h>
#include "6502_cpu.h"

uint16_t addr_imm(CPU *cpu, BUS *bus, uint8_t *page_cross);
uint16_t addr_zp(CPU *cpu, BUS *bus, uint8_t *page_cross);
uint16_t addr_zpx(CPU *cpu, BUS *bus, uint8_t *page_cross);
uint16_t addr_zpy(CPU *cpu, BUS *bus, uint8_t *page_cross);
uint16_t addr_abs(CPU *cpu, BUS *bus, uint8_t *page_cross);
uint16_t addr_absx(CPU *cpu, BUS *bus, uint8_t *page_cross);
uint16_t addr_absy(CPU *cpu, BUS *bus, uint8_t *page_cross);
uint16_t addr_indir(CPU *cpu, BUS *bus, uint8_t *page_cross);
uint16_t addr_indirx(CPU *cpu, BUS *bus, uint8_t *page_cross);
uint16_t addr_indiry(CPU *cpu, BUS *bus, uint8_t *page_cross);
uint16_t addr_rel(CPU *cpu, BUS *bus, uint8_t *page_cross);
uint16_t addr_imp(CPU *cpu, BUS *bus, uint8_t *page_cross);
// fill in the rest