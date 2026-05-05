/*
6502_addrmodes.c
TODO

*/

#include <stdint.h>
#include "6502_addrmodes.h"

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


//Returning address LOCATIONS - NOT DATA. CPUreads are getting data from locations @ address between $0000-$FFFF (16 bit) - 0-65355
uint16_t addr_imm(CPU *cpu,int *page_cross)     //Immediate
{
    return cpu->pc++;
}
uint16_t addr_zp(CPU *cpu,int *page_cross)      // Zeropage
{
    return cpu_read(cpu->pc++); // no hi byte, so just return this as uint16_t - will make highbyte zero
}
uint16_t addr_zpx(CPU *cpu,int *page_cross)     // Zeropage, X
{
    return (cpu_read(cpu->pc++) + cpu->x) & 0xFF; //page wrapping in zero page -> no cycle penalty
}
uint16_t addr_zpy(CPU *cpu,int *page_cross)     // Zeropage, Y
{
    return (cpu_read(cpu->pc++) + cpu->y) & 0xFF; //page wrapping in zero page -> no cycle penalty
}
uint16_t addr_abs(CPU *cpu,int *page_cross)     // Absolute
{
    uint16_t lo = cpu_read(cpu->pc++);
    uint16_t hi = cpu_read(cpu->pc++);
    return (hi << 8) | lo;  
}
uint16_t addr_absx(CPU *cpu,int *page_cross)    // Absolute,X
{
    uint16_t lo = cpu_read(cpu->pc++);
    uint16_t hi = cpu_read(cpu->pc++);

    uint16_t base = (hi << 8) | lo;
    uint16_t addr = base + cpu->x;

    *page_cross = ((base & 0xFF00) != (addr & 0xFF00));

    return addr;
}
uint16_t addr_absy(CPU *cpu,int *page_cross)    // Absolute,Y
{
    uint16_t lo = cpu_read(cpu->pc++);
    uint16_t hi = cpu_read(cpu->pc++);

    uint16_t base = (hi << 8) | lo;
    uint16_t addr = base + cpu->y;

    *page_cross = ((base & 0xFF00) != (addr & 0xFF00));
    return addr;
}
uint16_t addr_indirx(CPU *cpu,int *page_cross)  // Indirect, X
{
    uint16_t zp_addr = (cpu_read(cpu->pc++) + cpu->x) & 0xFF;
    uint16_t lo = cpu_read(zp_addr);
    uint16_t hi = cpu_read(zp_addr++);
    return (hi << 8) | lo;  
}
uint16_t addr_indiry(CPU *cpu,int *page_cross)  // Indirect, Y
{
    uint16_t zp_addr = cpu_read(cpu->pc++);

    uint16_t lo = cpu_read(zp_addr);
    uint16_t hi = cpu_read(zp_addr++);

    uint16_t base = (hi << 8) | lo;  
    uint16_t addr = base + cpu->y;

    *page_cross = ((base & 0xFF00) != (addr & 0xFF00));
    return addr;
}
