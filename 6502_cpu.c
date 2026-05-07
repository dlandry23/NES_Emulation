/*
6502_cpu.c
TODO: 
-----FUNCTIONS-----
initializeCPU registers
add read to reset vector
read memory/->fetch next op code -> pass this to main() where it runs it
*/

#include "6502_cpu.h"
#include "6502_opcodes.h"
#include "6502_addrmodes.h"

void cpu_init(CPU *cpu) {
    // TODO: initialize CPU state
    cpu->a = 0;
    cpu->x = 0;
    cpu->y = 0;
    cpu->s = 0xFD;
    cpu->p = 0x24;
    cpu->pc = 0;
}

/*
Addressing Modes Reference
---------------------------------------------------
addressing	    assembler	  bytes   cycles
---------------------------------------------------
immediate	     #oper	    	2	    2  
zeropage	     oper	    	2	    3  
zeropage,X	     oper,X	    	2	    4  
absolute	     oper	    	3	    4  
absolute,X	     oper,X	    	3	    4* 
absolute,Y	     oper,Y	    	3	    4* 
(indirect,X)	 (oper,X)		2	    6  
(indirect),Y     (oper),Y		2	    5* 
*/


Instruction table[256];

void init_table() {
    //ADC
    table[0x69] = (Instruction){ adc, addr_imm,     2};
    table[0x65] = (Instruction){ adc, addr_zp,      3};
    table[0x75] = (Instruction){ adc, addr_zpx,     4};
    table[0x6D] = (Instruction){ adc, addr_abs,     4};
    table[0x7F] = (Instruction){ adc, addr_absx,    4};
    table[0x79] = (Instruction){ adc, addr_absy,    4};
    table[0x61] = (Instruction){ adc, addr_indirx,  6};
    table[0x71] = (Instruction){ adc, addr_indiry,  5};

}

void cpu_step(CPU *cpu)
{
    int page_crossed =0;
    Instruction inst = table[opcode];
    uint16_t addr = inst.addrmode(cpu, &page_crossed);

    inst.operate(cpu, addr);
// THIS IS WRONG NOW?
    int cycles += inst.cycles;
    if (page_crossed && inst.add_cycle_on_page_cross) {
    cycles++;
}
};
