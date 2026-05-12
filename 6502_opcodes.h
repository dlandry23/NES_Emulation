/*
6502_opcodes.h
TODO
Add list of opcodes in format of void command(CPU *cpu,uint16_t addr) <- this is how it will be implemented in opcodes.c so this one can have
void command(CPU*, uint16_t) --- uint16_t because program counter is 16bits/2bytes

*/
#include <stdint.h>
#include "6502_cpu.h"//Access to CPU typedef stuct{}

//Transfer Instructions
void lda(CPU *cpu, uint16_t addr);
void ldx(CPU *cpu, uint16_t addr);
void ldy(CPU *cpu, uint16_t addr);
void sta(CPU *cpu, uint16_t addr);
void stx(CPU *cpu, uint16_t addr);
void sty(CPU *cpu, uint16_t addr);
void tax(CPU *cpu, uint16_t addr);
void tay(CPU *cpu, uint16_t addr);
void tsx(CPU *cpu, uint16_t addr);
void txa(CPU *cpu, uint16_t addr);
void txs(CPU *cpu, uint16_t addr);
void tya(CPU *cpu, uint16_t addr);

//Stack Instructions
void pha(CPU *cpu, BUS *bus);
void php(CPU *cpu, BUS *bus);
void pla(CPU *cpu, BUS *bus);
void plp(CPU *cpu, BUS *bus);
//Decrements & Increments
void dec(CPU *cpu, BUS *bus, uint16_t addr);
void dex(CPU *cpu);
void dey(CPU *cpu);
void inc(CPU *cpu, BUS *bus, uint16_t addr);
void inx(CPU *cpu);
void iny(CPU *cpu);
//Arithmetic
void adc(CPU *cpu, uint16_t addr); // Add with Carry
void sbc(CPU *cpu, uint16_t addr); // Subtract with Borrow
// CONTINUE ON DOWN FOR ALL (organize by function type to make it easier to read)