/*
6502_opcodes.c
TODO
Add actual functions considering the cpu and the addr
REMEMBER FOR ALL ARITHMATIC FUNCTIONS (ALU), THERE IS A BCD (Binary Coded Decimal) Flag which adds numbers together like decimals 
    this changes math implementations so cant just do simple adds - so $14 != 20, its $14=14 only affects ADC and SBC (NOT INC OR DEC)
*/
#include <stdio.h>
#include "6502_opcodes.h"
void adc(CPU *cpu, uint16_t addr) {
    // TODO: implement ADC
    
    printf("ADC not implemented yet\n");
}