/*
6502_opcodes.h
TODO
Add list of opcodes in format of void command(CPU *cpu,uint16_t addr) <- this is how it will be implemented in opcodes.c so this one can have
void command(CPU*, uint16_t) --- uint16_t because program counter is 16bits/2bytes

*/
#include <stdint.h>
#include "6502_cpu.h"//Access to CPU typedef stuct{}

// Transfer Instructions
void lda(CPU *cpu, BUS *bus, uint16_t addr);
void ldx(CPU *cpu, BUS *bus, uint16_t addr);
void ldy(CPU *cpu, BUS *bus, uint16_t addr);
void sta(CPU *cpu, BUS *bus, uint16_t addr);
void stx(CPU *cpu, BUS *bus, uint16_t addr);
void sty(CPU *cpu, BUS *bus, uint16_t addr);
void tax(CPU *cpu, BUS *bus, uint16_t addr);
void tay(CPU *cpu, BUS *bus, uint16_t addr);
void tsx(CPU *cpu, BUS *bus, uint16_t addr);
void txa(CPU *cpu, BUS *bus, uint16_t addr);
void txs(CPU *cpu, BUS *bus, uint16_t addr);
void tya(CPU *cpu, BUS *bus, uint16_t addr);
// Stack Instructions
void pha(CPU *cpu, BUS *bus, uint16_t addr);
void php(CPU *cpu, BUS *bus, uint16_t addr);
void pla(CPU *cpu, BUS *bus, uint16_t addr);
void plp(CPU *cpu, BUS *bus, uint16_t addr);
// Decrements & Increments
void dec(CPU *cpu, BUS *bus, uint16_t addr);
void dex(CPU *cpu, BUS *bus, uint16_t addr);
void dey(CPU *cpu, BUS *bus, uint16_t addr);
void inc(CPU *cpu, BUS *bus, uint16_t addr);
void inx(CPU *cpu, BUS *bus, uint16_t addr);
void iny(CPU *cpu, BUS *bus, uint16_t addr);
// Arithmetic
void adc(CPU *cpu, BUS *bus, uint16_t addr); // Add with Carry
void sbc(CPU *cpu, BUS *bus, uint16_t addr); // Subtract with Borrow
// Logical Operations
void and(CPU *cpu, BUS *bus, uint16_t addr);
void eor(CPU *cpu, BUS *bus, uint16_t addr);
void ora(CPU *cpu, BUS *bus, uint16_t addr);
// Shift & Rotate Instructions
void asl(CPU *cpu, BUS *bus, uint16_t addr);
void asl_a(CPU *cpu, BUS *bus, uint16_t addr); // SPECIAL CASE, JUST THE ACCUMULATOR
void lsr(CPU *cpu, BUS *bus, uint16_t addr);
void lsr_a(CPU *cpu, BUS *bus, uint16_t addr);// SPECIAL CASE, JUST THE ACCUMULATOR
void rol(CPU *cpu, BUS *bus, uint16_t addr);
void rol_a (CPU *cpu, BUS *bus, uint16_t addr);// SPECIAL CASE, JUST THE ACCUMULATOR
void ror(CPU *cpu, BUS *bus, uint16_t addr);
void ror_a(CPU *cpu, BUS *bus, uint16_t addr);// SPECIAL CASE, JUST THE ACCUMULATOR
// Flag Instructions
void clc(CPU *cpu, BUS *bus, uint16_t addr);
void cld(CPU *cpu, BUS *bus, uint16_t addr);
void cli(CPU *cpu, BUS *bus, uint16_t addr);
void clv(CPU *cpu, BUS *bus, uint16_t addr);
void sec(CPU *cpu, BUS *bus, uint16_t addr);
void sed(CPU *cpu, BUS *bus, uint16_t addr);
void sei(CPU *cpu, BUS *bus, uint16_t addr);
// Comparisons
void cmp(CPU *cpu, BUS *bus, uint16_t addr);
void cpx(CPU *cpu, BUS *bus, uint16_t addr);
void cpy(CPU *cpu, BUS *bus, uint16_t addr);
// Bit Test
void bit(CPU *cpu, BUS *bus, uint16_t addr);
//Conditional Branch Instructions
void bcc(CPU *cpu, BUS *bus, uint16_t addr);
void bcs(CPU *cpu, BUS *bus, uint16_t addr);
void beq(CPU *cpu, BUS *bus, uint16_t addr);
void bmi(CPU *cpu, BUS *bus, uint16_t addr);
void bne(CPU *cpu, BUS *bus, uint16_t addr);
void bpl(CPU *cpu, BUS *bus, uint16_t addr);
void bvc(CPU *cpu, BUS *bus, uint16_t addr);
void bvs(CPU *cpu, BUS *bus, uint16_t addr);
// Jumps and Subroutines
void jmp(CPU *cpu, BUS *bus, uint16_t addr);
void jsr(CPU *cpu, BUS *bus, uint16_t addr);
void rts(CPU *cpu, BUS *bus, uint16_t addr);
// Interrupts
void brk(CPU *cpu, BUS *bus, uint16_t addr);
void rti(CPU *cpu, BUS *bus, uint16_t addr);
// No Operation
void nop(CPU *cpu, BUS *bus, uint16_t addr);
// CONTINUE ON DOWN FOR ALL (organize by function type to make it easier to read)
void alr(CPU *cpu, BUS *bus, uint16_t addr);
void anc(CPU* cpu, BUS *bus, uint16_t addr);
void anc2(CPU* cpu, BUS *bus, uint16_t addr);
//void ane(CPU* cpu, BUS *bus, uint16_t addr);
//void arr(CPU* cpu, BUS *bus, uint16_t addr);
void dcp(CPU* cpu, BUS *bus, uint16_t addr);
void isb(CPU* cpu, BUS *bus, uint16_t addr);
//void las(CPU* cpu, BUS *bus, uint16_t addr);
void lax(CPU* cpu, BUS *bus, uint16_t addr);
//void lxa(CPU* cpu, BUS *bus, uint16_t addr);
void rla(CPU* cpu, BUS *bus, uint16_t addr);
void rra(CPU* cpu, BUS *bus, uint16_t addr);
void sax(CPU* cpu, BUS *bus, uint16_t addr);
//void sbx(CPU* cpu, BUS *bus, uint16_t addr);
//void sha(CPU* cpu, BUS *bus, uint16_t addr);
//void shx(CPU* cpu, BUS *bus, uint16_t addr);
//void shy(CPU* cpu, BUS *bus, uint16_t addr);
void slo(CPU* cpu, BUS *bus, uint16_t addr);
void sre(CPU* cpu, BUS *bus, uint16_t addr);
//void tas(CPU* cpu, BUS *bus, uint16_t addr);
void usbc(CPU* cpu, BUS *bus, uint16_t addr);