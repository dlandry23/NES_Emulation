/*
6502_process_flags.h

Define process status flags and quick inline function macros to read the bit, write the bit, toggle, set the bit, etc...

Status Register Flags (bit 7 to bit 0)
N	Negative
V	Overflow
-	ignored
B	Break
D	Decimal (use BCD for arithmetics)
I	Interrupt (IRQ disable)
Z	Zero
C	Carry

*/

#include <stdint.h>
#include <stdbool.h>

#ifndef CPU_FLAGS_H
#define CPU_FLAGS_H

#define FLAG_N 0x07 //                      Negative
#define FLAG_V 0x06 //                      Overflow
//THERE IS NO VALID FLAG IN POSITION 5
#define FLAG_5 0x05
#define FLAG_B 0x04 //                      Break
#define FLAG_D 0x03 //                      Decimal
#define FLAG_I 0x02 //                      Interrupt
#define FLAG_Z 0x01 //                      Zero
#define FLAG_C 0x00 //                      Carry

#define SET_FLAG(p,flag) ((p)|=(1u << (flag)))
#define CLR_FLAG(p,flag) ((p) &= ~(1u << (flag)))
#define TGL_FLAG(p,flag) ((p) ^= (1u << (flag)))
//READ-ONLY
#define GET_FLAG(p,flag) (((p) >> (flag)) & 1u)

#endif