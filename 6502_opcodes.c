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

static inline void set_zn(uint8_t *p, uint8_t data)
{
    (data & 0x80) ? SET_FLAG(*p,FLAG_N) : CLR_FLAG(*p, FLAG_N); 
    (data==0x00) ? SET_FLAG(*p,FLAG_Z) : CLR_FLAG(*p, FLAG_Z); 
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
void lda(CPU *cpu, BUS *bus, uint16_t addr)
{
    cpu->a = bus_read(bus,addr);
    set_zn(&cpu->p, cpu->a);
}
void ldx(CPU *cpu, BUS *bus, uint16_t addr)
{
    cpu->x = bus_read(bus,addr);
    set_zn(&cpu->p, cpu->x);
}
void ldy(CPU *cpu, BUS *bus, uint16_t addr)
{
    cpu->y = bus_read(bus,addr);
    set_zn(&cpu->p, cpu->y);
}
void sta(CPU *cpu, BUS *bus, uint16_t addr)
{
    
    bus_write(bus,addr,cpu->a);
}
void stx(CPU *cpu, BUS *bus, uint16_t addr)
{
    bus_write(bus,addr,cpu->x);
}
void sty(CPU *cpu, BUS *bus, uint16_t addr)
{
    bus_write(bus,addr,cpu->y);
}
void tax(CPU *cpu, BUS *bus, uint16_t addr)
{
    cpu->x = cpu->a;
    set_zn(&cpu->p, cpu->x);
}
void tay(CPU *cpu, BUS *bus, uint16_t addr)
{
    cpu->y = cpu->a;
    set_zn(&cpu->p, cpu->y);
}
void tsx(CPU *cpu, BUS *bus, uint16_t addr)
{
    cpu->x = cpu->s;
    set_zn(&cpu->p, cpu->x);
}
void txa(CPU *cpu, BUS *bus, uint16_t addr)
{
    cpu->a = cpu->x;
    set_zn(&cpu->p, cpu->a);
}
void txs(CPU *cpu, BUS *bus, uint16_t addr)
{
    cpu->s = cpu->x;
}
void tya(CPU *cpu, BUS *bus, uint16_t addr)
{
    cpu->a = cpu->y;
    set_zn(&cpu->p, cpu->a);
}

// Stack Instructions
/*
PHA - Push Accumulator onto stack
PHP - Push Processor Status Register tp stack (with break flag set)
PLA - Pull Accumulator off stack
PLP - Pull processor status register off stack
*/
void pha(CPU *cpu, BUS *bus, uint16_t addr)
{
    bus_write(bus,(0x0100 | cpu->s--),cpu->a); //(0x0100|cpu->s) -> Stack at location 0x0100-0x01FF - 256 bits set at page 1
    //cpu->s--;
}
void php(CPU *cpu, BUS *bus, uint16_t addr)
{
    SET_FLAG(cpu->p, FLAG_B);
    SET_FLAG(cpu->p, FLAG_5);
    bus_write(bus,(0x0100 | cpu->s),cpu->p);
    cpu->s--;
}
void pla(CPU *cpu, BUS *bus, uint16_t addr)
{
    cpu->a = bus_read(bus, 0x0100 | ++cpu->s);
    set_zn(&cpu->p, cpu->a);
}
void plp(CPU *cpu, BUS *bus, uint16_t addr)
{
    cpu->p = bus_read(bus, 0x0100 | ++cpu->s);
}

// Decrements & Increments
/*
DEC - Decrement Memory by 1
DEX - Decrement X by 1
DEY - Decrement Y by 1
INC - Increment Memory by 1
INX - Increment X by 1
INY - Increment Y by 1
*/
void dec(CPU *cpu, BUS *bus, uint16_t addr)
{
    uint8_t data = bus_read(bus,addr);
    data--;
    bus_write(bus,addr, data);
    set_zn(&cpu->p, data);
}
void dex(CPU *cpu, BUS *bus, uint16_t addr)
{
    cpu->x--;
    set_zn(&cpu->p, cpu->x);
}
void dey(CPU *cpu, BUS *bus, uint16_t addr)
{
    cpu->y--;
    set_zn(&cpu->p, cpu->y);
}
void inc(CPU *cpu, BUS *bus, uint16_t addr)
{
    uint8_t data = bus_read(bus,addr);
    data++;
    bus_write(bus,addr,data);
    set_zn(&cpu->p,data);
}
void inx(CPU *cpu, BUS *bus, uint16_t addr)
{
    cpu->x++;
    set_zn(&cpu->p, cpu->x);
}
void iny(CPU *cpu, BUS *bus, uint16_t addr)
{
    cpu->y++;
    set_zn(&cpu->p, cpu->y);
}

// Arithmatic Functions
/*
ADC - Add Memory to Accumulator with Carry
SBC - Subtract memory from Accumulator with Borrow
*/
void adc(CPU *cpu, BUS *bus, uint16_t addr) 
{
    uint8_t carry = GET_FLAG(cpu->p,FLAG_C);
    uint8_t value = bus_read(bus,addr);
    uint16_t sum = cpu->a + value + carry;

    //Overflow (V)
    ((~(cpu->a ^ value) & (cpu->a ^ sum) & 0x80) != 0) ? SET_FLAG(cpu->p,FLAG_V) : CLR_FLAG(cpu->p, FLAG_V);

    cpu->a = sum & 0xFF;
    //Status Flag Updates
    set_zn(&cpu->p,cpu->a);
    //Carry (C)
    (sum>0xFF) ? SET_FLAG(cpu->p,FLAG_C) : CLR_FLAG(cpu->p, FLAG_C); // Ternary Expression -> condition ? expression-true : expression-false

}
void sbc(CPU *cpu, BUS *bus, uint16_t addr)
{
    uint8_t carry = GET_FLAG(cpu->p,FLAG_C);
    uint8_t value = bus_read(bus, addr);
    uint16_t sum = (uint16_t)cpu->a + (uint8_t)(~value) + carry;

    //Overflow (V)
    (((cpu->a ^ sum) & (~value ^ sum) & 0x80) != 0) ? SET_FLAG(cpu->p,FLAG_V) : CLR_FLAG(cpu->p, FLAG_V);

    cpu->a = sum & 0xFF;
    //Status Flag Updates
    set_zn(&cpu->p,cpu->a);
    //Carry (C)
    (sum>0xFF) ? SET_FLAG(cpu->p,FLAG_C) : CLR_FLAG(cpu->p, FLAG_C); // Ternary Expression -> condition ? expression-true : expression-false
}

// Logical Operations
/*
AND - Memory & Accumulator
EOR - Memory ^ Accumulator (XOR)
ORA - Memory | Accumulator
*/
void and(CPU *cpu, BUS *bus, uint16_t addr)
{
    uint8_t data = bus_read(bus,addr);
    cpu->a = cpu->a & data;
    set_zn(&cpu->p, cpu->a);
}
void eor(CPU *cpu, BUS *bus, uint16_t addr)
{
    uint8_t data = bus_read(bus,addr);
    cpu->a = cpu->a ^ data;
    set_zn(&cpu->p, cpu->a);
}
void ora(CPU *cpu, BUS *bus, uint16_t addr)
{
    uint8_t data = bus_read(bus,addr);
    cpu->a = cpu->a | data;
    set_zn(&cpu->p, cpu->a);
}

// Shift & Rotate Instructions
/*
ASL - Arithmatic Shift Left
LSR - Logical Shift Right
ROL - Rotate Left 1 bit
ROR - Rotate Right 1 bit
*/
void asl(CPU *cpu, BUS *bus, uint16_t addr)
{
    uint8_t data = bus_read(bus,addr);
    (data & 0x80) ? SET_FLAG(cpu->p,FLAG_C)  : CLR_FLAG(cpu->p, FLAG_C); // Shifting left, capture bit 7 and store in carry flag

    data = data << 1;

    bus_write(bus,addr,data);
    set_zn(&cpu->p,data);
}
void asl_a(CPU *cpu, BUS *bus, uint16_t addr)   // SPECIAL CASE, JUST THE ACCUMULATOR
{
    (cpu->a & 0x80) ? SET_FLAG(cpu->p,FLAG_C)  : CLR_FLAG(cpu->p, FLAG_C);
    cpu->a = cpu->a << 1;
    set_zn(&cpu->p,cpu->a);
}
void lsr(CPU *cpu, BUS *bus, uint16_t addr)
{
    uint8_t data = bus_read(bus,addr);
    (data & 0x01) ? SET_FLAG(cpu->p,FLAG_C)  : CLR_FLAG(cpu->p, FLAG_C); // Shifting right, capture bit 0 and store in carry flag
    //CLR_FLAG(cpu->p, FLAG_N); // Shifting right in 0, always clear flag_N

    data = data >> 1;

    bus_write(bus,addr,data);
    set_zn(&cpu->p,data);
}
void lsr_a(CPU *cpu, BUS *bus, uint16_t addr)   // SPECIAL CASE, JUST THE ACCUMULATOR
{
    (cpu->a & 0x01) ? SET_FLAG(cpu->p,FLAG_C)  : CLR_FLAG(cpu->p, FLAG_C); // Shifting right, capture bit 0 and store in carry flag
    //CLR_FLAG(cpu->p, FLAG_N); // shifting in 0, always clear FLAG_N
    cpu->a = cpu->a >> 1;
    set_zn(&cpu->p,cpu->a);
}
void rol(CPU *cpu, BUS *bus, uint16_t addr)
{
    uint8_t data = bus_read(bus,addr);
    uint8_t old_carry = GET_FLAG(cpu->p,FLAG_C); // Store old carry, then record bit 7 to store in carry flag (0x00 or 0x01)
    (data & 0x80) ? SET_FLAG(cpu->p,FLAG_C)  : CLR_FLAG(cpu->p, FLAG_C); // Shifting left, capture bit 7 and store in carry flag

    data = (data << 1) | old_carry;

    bus_write(bus,addr,data);
    set_zn(&cpu->p,data);
}
void rol_a (CPU *cpu, BUS *bus, uint16_t addr)  // SPECIAL CASE, JUST THE ACCUMULATOR
{
    uint8_t old_carry = GET_FLAG(cpu->p,FLAG_C); // Store old carry, then record bit 7 to store in carry flag (0x00 or 0x01)
    (cpu->a & 0x80) ? SET_FLAG(cpu->p,FLAG_C)  : CLR_FLAG(cpu->p, FLAG_C); // Shifting left, capture bit 7 and store in carry flag

    cpu->a = (cpu->a << 1) | old_carry;

    set_zn(&cpu->p,cpu->a);
}
void ror(CPU *cpu, BUS *bus, uint16_t addr)
{
    uint8_t data = bus_read(bus,addr);
    uint8_t old_carry = GET_FLAG(cpu->p,FLAG_C); // Store old carry, then record bit 7 to store in carry flag (0x00 or 0x01)
    (data & 0x01) ? SET_FLAG(cpu->p,FLAG_C)  : CLR_FLAG(cpu->p, FLAG_C); // Shifting right, capture bit 0 and store in carry flag

    data = (data >> 1) | (old_carry << 7);

    bus_write(bus,addr,data);
    set_zn(&cpu->p,data);
}
void ror_a(CPU *cpu, BUS *bus, uint16_t addr)   // SPECIAL CASE, JUST THE ACCUMULATOR
{
    uint8_t old_carry = GET_FLAG(cpu->p,FLAG_C); // Store old carry, then record bit 7 to store in carry flag (0x00 or 0x01)
    (cpu->a & 0x01) ? SET_FLAG(cpu->p,FLAG_C)  : CLR_FLAG(cpu->p, FLAG_C); // Shifting right, capture bit 0 and store in carry flag

    cpu->a = (cpu->a >> 1) | (old_carry << 7);

    set_zn(&cpu->p,cpu->a);
}

// Flag Instructions
/*
CLC - Clear Carry
CLD - Clear Decimal
CLI - Clear Interrupt Disable
CLV - Clear Overflow
SEC - Set Carry
SED - Set Decimal
SEI - Set Interrupt Disable
*/
void clc(CPU *cpu, BUS *bus, uint16_t addr)
{
    CLR_FLAG(cpu->p,FLAG_C);
}
void cld(CPU *cpu, BUS *bus, uint16_t addr)
{
    CLR_FLAG(cpu->p,FLAG_D);
}
void cli(CPU *cpu, BUS *bus, uint16_t addr)
{
    CLR_FLAG(cpu->p,FLAG_I);
}
void clv(CPU *cpu, BUS *bus, uint16_t addr)
{
    CLR_FLAG(cpu->p,FLAG_V);
}
void sec(CPU *cpu, BUS *bus, uint16_t addr)
{
    SET_FLAG(cpu->p,FLAG_C);
}
void sed(CPU *cpu, BUS *bus, uint16_t addr)
{
    SET_FLAG(cpu->p,FLAG_D);
}
void sei(CPU *cpu, BUS *bus, uint16_t addr)
{
    SET_FLAG(cpu->p,FLAG_I);
}

// Comparisons
/*
CMP - Compare Memory with Accumulator
CPX - Compare Memory with X
CPY - Compare Memory with Y
*/
void cmp(CPU *cpu, BUS *bus, uint16_t addr)
{
    uint8_t data = bus_read(bus,addr);
    uint16_t sum = (uint16_t)cpu->a + (uint8_t)(~data) + 1;
    set_zn(&cpu->p, sum & 0xFF);
    (sum>0xFF) ? SET_FLAG(cpu->p,FLAG_C) : CLR_FLAG(cpu->p, FLAG_C); // Ternary Expression -> condition ? expression-true : expression-false
}
void cpx(CPU *cpu, BUS *bus, uint16_t addr)
{
    uint8_t data = bus_read(bus,addr);
    uint16_t sum = (uint16_t)cpu->x + (uint8_t)(~data) + 1;
    set_zn(&cpu->p, sum & 0xFF);
    (sum>0xFF) ? SET_FLAG(cpu->p,FLAG_C) : CLR_FLAG(cpu->p, FLAG_C); // Ternary Expression -> condition ? expression-true : expression-false
}
void cpy(CPU *cpu, BUS *bus, uint16_t addr)
{
    uint8_t data = bus_read(bus,addr);
    uint16_t sum = (uint16_t)cpu->y + (uint8_t)(~data) + 1;
    set_zn(&cpu->p, sum & 0xFF);
    (sum>0xFF) ? SET_FLAG(cpu->p,FLAG_C) : CLR_FLAG(cpu->p, FLAG_C); // Ternary Expression -> condition ? expression-true : expression-false
}

// Bit Test
/*
BIT - A & M - use result to set FLAG_Z, M7 -> FLAG_N, M6 -> FLAG_V
*/
void bit(CPU *cpu, BUS *bus, uint16_t addr)
{
    uint8_t data = bus_read(bus,addr);
    uint8_t result = cpu->a & data;
    (result==0x00) ? SET_FLAG(cpu->p,FLAG_Z) : CLR_FLAG(cpu->p, FLAG_Z);
    (data & 0x80) ? SET_FLAG(cpu->p,FLAG_N) : CLR_FLAG(cpu->p, FLAG_N);
    (data & 0x40) ? SET_FLAG(cpu->p,FLAG_V) : CLR_FLAG(cpu->p, FLAG_V);
}

// Conditional Branch Instructions
/*
BCC - Branch on Carry Clear
BCS - on Carry Set
BEQ - on Equal (Zero Flag Set)
BMI - on Minus (Negative Flag Set)
BNE - on not equal (Zero Flag Clear)
BPL - on Plus (Negative Flag Clear)
BVC - on Overflow Clear
BVS - on Overflow Set
// offset calculations handled in addr_rel
*/
void bcc(CPU *cpu, BUS *bus, uint16_t addr)
{
    
    if (!GET_FLAG(cpu->p,FLAG_C)) 
    {
        uint16_t base = cpu->pc;
        cpu->pc = addr;
        cpu->cycles++;
        if ((base & 0xFF00) != (addr & 0xFF00)) cpu->cycles++;
    }
    
    
}
void bcs(CPU *cpu, BUS *bus, uint16_t addr)
{
    if (GET_FLAG(cpu->p,FLAG_C))
    {
        uint16_t base = cpu->pc;
        cpu->pc = addr;
        cpu->cycles++;
        if ((base & 0xFF00) != (addr & 0xFF00)) cpu->cycles++;
    }
}
void beq(CPU *cpu, BUS *bus, uint16_t addr)
{
    if (GET_FLAG(cpu->p,FLAG_Z))
    {
        uint16_t base = cpu->pc;
        cpu->pc = addr;
        cpu->cycles++;
        if ((base & 0xFF00) != (addr & 0xFF00)) cpu->cycles++;
    }
}
void bmi(CPU *cpu, BUS *bus, uint16_t addr)
{
    if (GET_FLAG(cpu->p,FLAG_N))
    {
        uint16_t base = cpu->pc;
        cpu->pc = addr;
        cpu->cycles++;
        if ((base & 0xFF00) != (addr & 0xFF00)) cpu->cycles++;
    }
}
void bne(CPU *cpu, BUS *bus, uint16_t addr)
{
    if (!GET_FLAG(cpu->p,FLAG_Z))
    {
        uint16_t base = cpu->pc;
        cpu->pc = addr;
        cpu->cycles++;
        if ((base & 0xFF00) != (addr & 0xFF00)) cpu->cycles++;
    }
}
void bpl(CPU *cpu, BUS *bus, uint16_t addr)
{
    if (!GET_FLAG(cpu->p,FLAG_N))
    {
        uint16_t base = cpu->pc;
        cpu->pc = addr;
        cpu->cycles++;
        if ((base & 0xFF00) != (addr & 0xFF00)) cpu->cycles++;
    }
}
void bvc(CPU *cpu, BUS *bus, uint16_t addr)
{
    if (!GET_FLAG(cpu->p,FLAG_V))
    {
        uint16_t base = cpu->pc;
        cpu->pc = addr;
        cpu->cycles++;
        if ((base & 0xFF00) != (addr & 0xFF00)) cpu->cycles++;
    }
}
void bvs(CPU *cpu, BUS *bus, uint16_t addr)
{
    if (GET_FLAG(cpu->p,FLAG_V))
    {
        uint16_t base = cpu->pc;
        cpu->pc = addr;
        cpu->cycles++;
        if ((base & 0xFF00) != (addr & 0xFF00)) cpu->cycles++;
    }
}

// Jumps and Subroutines
/*
JMP - Jump to address
JSR - Jump to subroutine
RTS - Return from subroutine
*/
void jmp(CPU *cpu, BUS *bus, uint16_t addr)
{
    cpu->pc = addr;
}
void jsr(CPU *cpu, BUS *bus, uint16_t addr)
{
    uint8_t hi = (uint8_t)(((cpu->pc - 1) & 0XFF00)>>8);
    uint8_t lo = (uint8_t)(((cpu->pc - 1) & 0X00FF));
    bus_write(bus,(0x0100 | cpu->s--),hi);
    bus_write(bus,(0x0100 | cpu->s--),lo);
    cpu->pc = addr;
}
void rts(CPU *cpu, BUS *bus, uint16_t addr)
{
    uint16_t lo = (uint16_t)bus_read(bus,(0x0100 | ++cpu->s));
    uint16_t hi = (uint16_t)bus_read(bus,(0x0100 | ++cpu->s));
    cpu->pc = ((hi << 8) | lo)+ 0x0001;
}

// Interrupts
/*
BRK - Break, go to IRQ vector, push pc, then push P, set I, jump to $FFFE
RTI - Return from Interrupt
*/
void brk(CPU *cpu, BUS *bus, uint16_t addr)
{
    cpu->pc++;

    bus_write(bus,(0x0100 | cpu->s--),((cpu->pc >> 8) & 0xFF));
    bus_write(bus,(0x0100 | cpu->s--),((cpu->pc) & 0xFF));
    SET_FLAG(cpu->p,FLAG_B);
    bus_write(bus,(0x0100 | cpu->s--),cpu->p);

    SET_FLAG(cpu->p,FLAG_I);

    uint16_t lo = bus_read(bus, 0xFFFE);
    uint16_t hi = bus_read(bus, 0xFFFF);
    cpu->pc = (hi << 8) | lo;
}
void rti(CPU *cpu, BUS *bus, uint16_t addr)
{
    cpu->p = bus_read(bus,(0x0100 | ++cpu->s));
    CLR_FLAG(cpu->p, FLAG_B);
    uint16_t lo = (uint16_t)bus_read(bus,(0x0100 | ++cpu->s));
    uint16_t hi = (uint16_t)bus_read(bus,(0x0100 | ++cpu->s));

    cpu->pc = ((hi << 8) | lo);
}


// No Operation
void nop(CPU *cpu, BUS *bus, uint16_t addr)
{
    // Much ado about nothing
}

// Illegal OPCODES
void alr(CPU *cpu, BUS *bus, uint16_t addr) //AND with OPERAND, then LSR
{
    uint8_t data = bus_read(bus,addr);
    cpu->a = cpu->a & data;
    set_zn(&cpu->p, cpu->a);

    (cpu->a & 0x01) ? SET_FLAG(cpu->p,FLAG_C)  : CLR_FLAG(cpu->p, FLAG_C); // Shifting right, capture bit 0 and store in carry flag
    //CLR_FLAG(cpu->p, FLAG_N); // shifting in 0, always clear FLAG_N
    cpu->a = cpu->a >> 1;
    set_zn(&cpu->p,cpu->a);
}

void anc(CPU* cpu, BUS *bus, uint16_t addr) // AND + set C as ASL
{
    uint8_t data = bus_read(bus,addr);
    cpu->a = cpu->a & data;
    set_zn(&cpu->p, cpu->a);
    (data & 0x80) ? SET_FLAG(cpu->p,FLAG_C)  : CLR_FLAG(cpu->p, FLAG_C);
}
void anc2(CPU* cpu, BUS *bus, uint16_t addr)// AND + set C as ROL -- identical to anc
{
    uint8_t data = bus_read(bus,addr);
    cpu->a = cpu->a & data;
    set_zn(&cpu->p, cpu->a);
    (data & 0x80) ? SET_FLAG(cpu->p,FLAG_C)  : CLR_FLAG(cpu->p, FLAG_C);
}
//void ane(CPU* cpu, BUS *bus, uint16_t addr); unstable don't implement
//void arr(CPU* cpu, BUS *bus, uint16_t addr); NOT IN NESTEST
void dcp(CPU* cpu, BUS *bus, uint16_t addr) // Decrement, then compare to accumulator
{
    uint8_t data = bus_read(bus,addr);
    data--;
    bus_write(bus,addr, data);
    set_zn(&cpu->p, data);

    uint16_t sum = (uint16_t)cpu->a + (uint8_t)(~data) + 1;
    set_zn(&cpu->p, sum & 0xFF);
    (sum>0xFF) ? SET_FLAG(cpu->p,FLAG_C) : CLR_FLAG(cpu->p, FLAG_C); // Ternary Expression -> condition ? expression-true : expression-false
}

void isb(CPU* cpu, BUS *bus, uint16_t addr) //increment the operator, then sbc oper M + 1 -> M, A - M - C̅ -> A
{
    uint8_t data = bus_read(bus,addr);
    data++;
    bus_write(bus,addr,data);
    set_zn(&cpu->p,data);

    uint8_t carry = GET_FLAG(cpu->p,FLAG_C);
    uint16_t sum = (uint16_t)cpu->a + (uint8_t)(~data) + carry;

    //Overflow (V)
    (((cpu->a ^ sum) & (~data ^ sum) & 0x80) != 0) ? SET_FLAG(cpu->p,FLAG_V) : CLR_FLAG(cpu->p, FLAG_V);

    cpu->a = sum & 0xFF;
    //Status Flag Updates
    set_zn(&cpu->p,cpu->a);
    //Carry (C)
    (sum>0xFF) ? SET_FLAG(cpu->p,FLAG_C) : CLR_FLAG(cpu->p, FLAG_C); // Ternary Expression -> condition ? expression-true : expression-false
}
//void las(CPU* cpu, BUS *bus, uint16_t addr); not implemented
void lax(CPU* cpu, BUS *bus, uint16_t addr) // LDA  then LDX
{
    cpu->a = bus_read(bus,addr);
    set_zn(&cpu->p, cpu->a);

    cpu->x = bus_read(bus,addr);
    set_zn(&cpu->p, cpu->x);
}
//void lxa(CPU* cpu, BUS *bus, uint16_t addr); not implemented
void rla(CPU* cpu, BUS *bus, uint16_t addr) // ROL + AND
{
    uint8_t data = bus_read(bus,addr);
    uint8_t old_carry = GET_FLAG(cpu->p,FLAG_C); // Store old carry, then record bit 7 to store in carry flag (0x00 or 0x01)
    (data & 0x80) ? SET_FLAG(cpu->p,FLAG_C)  : CLR_FLAG(cpu->p, FLAG_C); // Shifting left, capture bit 7 and store in carry flag

    data = (data << 1) | old_carry;

    bus_write(bus,addr,data);
    set_zn(&cpu->p,data);

    cpu->a = cpu->a & data;
    set_zn(&cpu->p, cpu->a);

}
void rra(CPU* cpu, BUS *bus, uint16_t addr) // ROR + ADC
{
    uint8_t data = bus_read(bus,addr);
    uint8_t old_carry = GET_FLAG(cpu->p,FLAG_C); // Store old carry, then record bit 7 to store in carry flag (0x00 or 0x01)
    (data & 0x01) ? SET_FLAG(cpu->p,FLAG_C)  : CLR_FLAG(cpu->p, FLAG_C); // Shifting right, capture bit 0 and store in carry flag

    data = (data >> 1) | (old_carry << 7);

    bus_write(bus,addr,data);
    set_zn(&cpu->p,data);

    uint8_t carry = GET_FLAG(cpu->p,FLAG_C);
    uint16_t sum = cpu->a + data + carry;

    //Overflow (V)
    ((~(cpu->a ^ data) & (cpu->a ^ sum) & 0x80) != 0) ? SET_FLAG(cpu->p,FLAG_V) : CLR_FLAG(cpu->p, FLAG_V);

    cpu->a = sum & 0xFF;
    //Status Flag Updates
    set_zn(&cpu->p,cpu->a);
    //Carry (C)
    (sum>0xFF) ? SET_FLAG(cpu->p,FLAG_C) : CLR_FLAG(cpu->p, FLAG_C); // Ternary Expression -> condition ? expression-true : expression-false

}
void sax(CPU* cpu, BUS *bus, uint16_t addr) // actually new -> A and X stored at in M
{
    uint8_t data = cpu->a & cpu->x;
    bus_write(bus,addr,data);
}
//void sbx(CPU* cpu, BUS *bus, uint16_t addr);
//void sha(CPU* cpu, BUS *bus, uint16_t addr);
//void shx(CPU* cpu, BUS *bus, uint16_t addr);
//void shy(CPU* cpu, BUS *bus, uint16_t addr);
void slo(CPU* cpu, BUS *bus, uint16_t addr) // ASL + ORA
{
    uint8_t data = bus_read(bus,addr);
    (data & 0x80) ? SET_FLAG(cpu->p,FLAG_C)  : CLR_FLAG(cpu->p, FLAG_C); // Shifting left, capture bit 7 and store in carry flag

    data = data << 1;

    bus_write(bus,addr,data);
    set_zn(&cpu->p,data);


    cpu->a = cpu->a | data;
    set_zn(&cpu->p, cpu->a);
}
void sre(CPU* cpu, BUS *bus, uint16_t addr) // LSR + EOR
{
    uint8_t data = bus_read(bus,addr);
    (data & 0x01) ? SET_FLAG(cpu->p,FLAG_C)  : CLR_FLAG(cpu->p, FLAG_C); // Shifting right, capture bit 0 and store in carry flag
    //CLR_FLAG(cpu->p, FLAG_N); // Shifting right in 0, always clear flag_N

    data = data >> 1;

    bus_write(bus,addr,data);
    set_zn(&cpu->p,data);

    cpu->a = cpu->a ^ data;
    set_zn(&cpu->p, cpu->a);
}
//void tas(CPU* cpu, BUS *bus, uint16_t addr);
//void usbc(CPU* cpu, BUS *bus, uint16_t addr) //Equal to SBC immediate instr. E9 - will deal with it that way in the table...