/*
6502_opcodes.c
TODO
Add actual functions considering the cpu and the addr
REMEMBER FOR ALL ARITHMATIC FUNCTIONS (ALU), THERE IS A BCD (Binary Coded Decimal) Flag which adds numbers together like decimals 
    this changes math implementations so cant just do simple adds - so $14 != 20, its $14=14 only affects ADC and SBC (NOT INC OR DEC)
*/
#include <stdio.h>
#include "6502_opcodes.h"
#include "6502_process_flags.h"

/*
Updating the Process Status Register (p)
Negative        (N) -> 7 bit in ADC ==1
Overflow        (V) -> result of signed math outside of -128->+127
Zero            (Z) -> Accumulator = Zero 
Carry           (C) -> See if uint16_t sum contains values in hi-byte
*/

static inline void set_zn(uint8_t *p)
{
    (*p & 0x80) ? SET_FLAG(*p,FLAG_N) : CLR_FLAG(*p, FLAG_N); 
    ((*p)==0x00) ? SET_FLAG(*p,FLAG_Z) : CLR_FLAG(*p, FLAG_Z); 
    /*
    //Negative (N)
    (((cpu->a) >>0x07) & 1u) ? SET_FLAG(cpu->p,FLAG_N) : CLR_FLAG(cpu->p, FLAG_N); //Check if 1 in bit 7 of A
    //Zero (Z)
    ((cpu->a)==0x00) ? SET_FLAG(cpu->p,FLAG_Z) : CLR_FLAG(cpu->p, FLAG_Z); 
    */
}

// Transfer Functions
/*
LDA - Load Accumulator with memory
LDX - Load Index X with memory
LDY - Load Index Y with Memory
STA - Store Accumulator to memory
STX - Store X to memory
STY - Store Y to memory
TAX - Transfer Accumulator to X
TAY - Transfer Accumulator to Y
TSX - Transfer Stack Pointer (S) to X
TXA - Transfer X to Accumulator
TXS - Transfer X to Stack Pointer (S)
TYA - Transfer Y to Accumulator
*/
void lda(CPU *cpu, uint16_t addr)
{
    cpu->a = cpu_read(addr);
    set_zn(cpu->p);
}
void ldx(CPU *cpu, uint16_t addr)
{
    cpu->x = cpu_read(addr);
    set_zn(cpu->p);
}
void ldy(CPU *cpu, uint16_t addr)
{
    cpu->y = cpu_read(addr);
    set_zn(cpu->p);
}
void sta(CPU *cpu, uint16_t addr)
{
    cpu_write(addr,cpu->a);
}
void stx(CPU *cpu, uint16_t addr)
{
    cpu_write(addr,cpu->x);
}
void sty(CPU *cpu, uint16_t addr)
{
    cpu_write(addr,cpu->y);
}
void tax(CPU *cpu, uint16_t addr)
{
    cpu->x = cpu->a;
    set_zn(cpu->p);
}
void tay(CPU *cpu, uint16_t addr)
{
    cpu->y = cpu->a;
    set_zn(cpu->p);
}
void tsx(CPU *cpu, uint16_t addr)
{
    cpu->x = cpu->s;
    set_zn(cpu->p);
}
void txa(CPU *cpu, uint16_t addr)
{
    cpu->a = cpu->x;
    set_zn(cpu->p);
}
void txs(CPU *cpu, uint16_t addr)
{
    cpu->s = cpu->x;
    set_zn(cpu->p);
}
void tya(CPU *cpu, uint16_t addr)
{
    cpu->a = cpu->y;
    set_zn(cpu->p);
}

// Stack Instructions
/*
PHA - Push Accumulator onto stack
PHP - Push Processor Status Register tp stack (with break flag set)
PLA - Pull Accumulator off stack
PLP - Pull processor status register off stack
*/
void pha(CPU *cpu)
{
    write_cpu((0x0100|cpu->s),cpu->a); //(0x0100|cpu->s) -> Stack at location 0x0100-0x01FF - 256 bits set at page 1
    cpu->s--;
}
void php(CPU *cpu)
{
    write_cpu((0x0100|cpu->s),cpu->a);
    cpu->s--;
}
void pla(CPU *cpu)
{
    cpu->a = cpu_read(cpu->s);
    cpu->s++;
}
void plp(CPU *cpu)
{
    cpu->p = cpu_read(cpu->s);
    cpu->s++;
}
// Arithmatic Functions
/*
ADC - Add Memory to Accumulator with Carry
SBC - Subtract memory from Accumulator with Borrow
*/
void adc(CPU *cpu, uint16_t addr) 
{
    uint8_t carry = GET_FLAG(cpu->p,FLAG_C);
    uint8_t value = cpu_read(addr);
    uint16_t sum = cpu->a + value + carry;

    //Overflow (V)
    ((~(cpu->a ^ value) & (cpu->a ^ sum) & 0x80) != 0) ? SET_FLAG(cpu->p,FLAG_V) : CLR_FLAG(cpu->p, FLAG_V);

    cpu->a = sum & 0xFF;
    //Status Flag Updates
    set_zn(cpu->p);
    //Carry (C)
    (sum>0xFF) ? SET_FLAG(cpu->p,FLAG_C) : CLR_FLAG(cpu->p, FLAG_C); // Ternary Expression -> condition ? expression-true : expression-false

}
void sbc(CPU *cpu, uint16_t addr)
{
    uint8_t carry = GET_FLAG(cpu->p,FLAG_C);
    uint8_t value = cpu_read(addr);
    uint16_t sum = cpu->a + ~(value) + carry;

    //Overflow (V)
    ((~(cpu->a ^ sum) & (~value ^ sum) & 0x80) != 0) ? SET_FLAG(cpu->p,FLAG_V) : CLR_FLAG(cpu->p, FLAG_V);

    cpu->a = sum & 0xFF;
    //Status Flag Updates
    set_zn(cpu->p);
    //Carry (C)
    (sum>0xFF) ? SET_FLAG(cpu->p,FLAG_C) : CLR_FLAG(cpu->p, FLAG_C); // Ternary Expression -> condition ? expression-true : expression-false
}

//Bit Test

//Comparisons

//Conditional Branch Instructions

// Decrements & Increments

//