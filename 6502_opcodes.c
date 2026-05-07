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


//Arithmatic Functions
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
    //Negative (N)
    (((cpu->a) >>0x07) & 1u) ? SET_FLAG(cpu->p,FLAG_N) : CLR_FLAG(cpu->p, FLAG_N); //Check if 1 in bit 7 of A
    //Zero (Z)
    ((cpu->a)==0x00) ? SET_FLAG(cpu->p,FLAG_Z) : CLR_FLAG(cpu->p, FLAG_Z); 
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
    //Negative (N)
    (((cpu->a) >>0x07) & 1u) ? SET_FLAG(cpu->p,FLAG_N) : CLR_FLAG(cpu->p, FLAG_N); //Check if 1 in bit 7 of A
    //Zero (Z)
    ((cpu->a)==0x00) ? SET_FLAG(cpu->p,FLAG_Z) : CLR_FLAG(cpu->p, FLAG_Z); 
    //Carry (C)
    (sum>0xFF) ? SET_FLAG(cpu->p,FLAG_C) : CLR_FLAG(cpu->p, FLAG_C); // Ternary Expression -> condition ? expression-true : expression-false
}