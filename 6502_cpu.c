/*
6502_cpu.c
TODO: 
-----FUNCTIONS-----
initializeCPU registers
add read to reset vector
read memory/->fetch next op code -> pass this to main() where it runs it
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "6502_cpu.h"
#include "6502_opcodes.h"
#include "6502_addrmodes.h"
#include "NES_ppu.h"
#include "NES_apu.h"
#include "NES_mapper.h"
#include "NES_file.h"

void cpu_init(CPU *cpu) {
    // TODO: initialize CPU state
    cpu->a = 0;
    cpu->x = 0;
    cpu->y = 0;
    cpu->s = 0xFD; // top of the stack on reset
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
    // LDA
    table[0xA9] = (Instruction){ lda, addr_imm,     2};
    table[0xA5] = (Instruction){ lda, addr_zp,      3};
    table[0xB5] = (Instruction){ lda, addr_zpx,     4};
    table[0xAD] = (Instruction){ lda, addr_abs,     4};
    table[0xBD] = (Instruction){ lda, addr_absx,    4};
    table[0xB9] = (Instruction){ lda, addr_absy,    4};
    table[0xA1] = (Instruction){ lda, addr_indirx,  6};
    table[0xB1] = (Instruction){ lda, addr_indiry,  5};
    // LDX
    table[0xA2] = (Instruction){ ldx, addr_imm, 2};
    table[0xA6] = (Instruction){ ldx, addr_zp, 3};
    table[0xB6] = (Instruction){ ldx, addr_zpy, 4};
    table[0xAE] = (Instruction){ ldx, addr_abs, 4};
    table[0xBE] = (Instruction){ ldx, addr_absy, 4};
    // LDY
    table[0xA0] = (Instruction){ ldy, addr_imm, 2};
    table[0xA4] = (Instruction){ ldy, addr_zp, 3};
    table[0xB4] = (Instruction){ ldy, addr_zpx, 4};
    table[0xAC] = (Instruction){ ldy, addr_abs, 4};
    table[0xBC] = (Instruction){ ldy, addr_absx, 4};
    // STA
    table[0x85] = (Instruction){ sta, addr_zp, 3};
    table[0x95] = (Instruction){ sta, addr_zpx, 4};
    table[0x8D] = (Instruction){ sta, addr_abs, 4};
    table[0x9D] = (Instruction){ sta, addr_absx, 5};
    table[0x99] = (Instruction){ sta, addr_absy, 5};
    table[0x81] = (Instruction){ sta, addr_indirx, 6};
    table[0x91] = (Instruction){ sta, addr_indiry, 6};
    // STX
    table[0x86] = (Instruction){ stx, addr_zp, 3};
    table[0x96] = (Instruction){ stx, addr_zpy, 4};
    table[0x8E] = (Instruction){ stx, addr_abs, 4};
    // STY
    table[0x84] = (Instruction){ sty, addr_zp, 3};
    table[0x94] = (Instruction){ sty, addr_zpx, 4};
    table[0x8C] = (Instruction){ sty, addr_abs, 4};
    // TAX
    table[0x8A] = (Instruction){ txa, addr_imp, 2};
    //ADC
    table[0x69] = (Instruction){ adc, addr_imm,     2};
    table[0x65] = (Instruction){ adc, addr_zp,      3};
    table[0x75] = (Instruction){ adc, addr_zpx,     4};
    table[0x6D] = (Instruction){ adc, addr_abs,     4};
    table[0x7F] = (Instruction){ adc, addr_absx,    4};
    table[0x79] = (Instruction){ adc, addr_absy,    4};
    table[0x61] = (Instruction){ adc, addr_indirx,  6};
    table[0x71] = (Instruction){ adc, addr_indiry,  5};

    // AND
    table[0x29] = (Instruction){ and, addr_imm, 2};
    table[0x25] = (Instruction){ and, addr_zp, 2};
    table[0x35] = (Instruction){ and, addr_zpx, 2};
    table[0x2D] = (Instruction){ and, addr_abs, 3};
    table[0x3D] = (Instruction){ and, addr_absx, 3};
    table[0x39] = (Instruction){ and, addr_absy, 3};
    table[0x21] = (Instruction){ and, addr_indirx, 2};
    table[0x31] = (Instruction){ and, addr_indiry, 2};


}






    /*//uint8_t *buffer = &state->memory[offset]; //Reference the memory location of state as buffer, fread into the "buffer" - use to read what is in the memory banks (particularly just last bank matters)
    fread(buffer_rom,1,fsize,f);
    fclose(f);
    uint8_t prg_bnk_num = buffer_rom[4];
    uint8_t chr_bnk_num = buffer_rom[5];
    memcpy(&state->memory[offset],&buffer_rom[(0x4000*(prg_bnk_num-0x4))+0x10],0x10000);
    //return buffer_rom;*/
//Initialize Banks
void init_banks (BUS *bus)
{
    bus ->prg_banks[0];
}
//CPU STEP

void cpu_step(CPU *cpu, BUS *bus)
{
    int page_crossed =0;
    uint8_t opcode = bus_read(bus,cpu->pc++); // Read the current byte (op_code), prepare to read the next byte (use addressing modes)
    Instruction inst = table[opcode]; // find opcode in the table
    uint16_t addr = inst.addrmode(cpu, bus, &page_crossed); // address according to addressing modes 
    cpu->cycles      = inst.cycles;
    cpu->cycles     += page_crossed;
    inst.operate(cpu,bus,addr); // tick up the cycles if need

}

// BUS READ / BUS WRITE

uint8_t bus_read (BUS *bus, uint16_t addr)
{
    if (addr < 0x2000) //RAM - mirrors every 0x0800
    {
        return bus->ram[addr & 0x07FF];
    }
    else if (addr < 0x4000) //PPU - registers mirrored every 8 bytes
    {
        return ppu_read(bus->ppu,0x2000 + (addr & 0x0007));
    }
    else if (addr < 0x4018) //APU and I/O registers
    {
        return apu_read(bus->apu,addr);
    }
    else if (addr >=0x8000) //PRG_ROM
    {
        uint8_t bank_index = (addr-0x8000) / 0x2000; //0,1,2,3
        uint16_t offset = addr & 0x1FFF;
        return bus->prg_banks[bank_index][offset];  // give the address of what is currently there in rom 
                                // (may need to take this out of "Bus" - as it may not be exactly represented accurately)
    }
    return 0x00; // open bus behaviour
}

void bus_write (BUS *bus, uint16_t addr, uint8_t data)
{
    if (addr < 0x2000)
    {
        bus->ram[addr & 0x07FF] = data;
    }
    else if (addr < 0x4000) //PPU - registers mirrored every 8 bytes
    {
        ppu_write(bus->ppu,0x2000 + (addr & 0x0007),data);
    }
    else if (addr < 0x4018) //APU and I/O registers
    {
        apu_write(bus->apu,addr,data);
    }
    else if (addr >=0x8000) //PRG_ROM mapper registers for MMC3 - 
    // actually limiting the top end does not matter even though only banks up to and including $E001 are important - Nothing will get written up 
    // here by actual games unless its for a bank switch operation
    {
        switch (addr & 0xE001) 
        {
            case 0x8000: // bank select
                bus->mapper.bank_select = data;
                break;
            case 0x8001: // bank data
                // handle bank switching
                bank_switch(bus,data);
                break;
            case 0xA000: // mirroring
                break;
            case 0xA001: // PRG RAM protect
                break;
            case 0xC000: // IRQ latch
                break;
            case 0xC001: // IRQ reload
                break;
            case 0xE000: // IRQ disable
                break;
            case 0xE001: // IRQ enable
                break;
        }
    }

}

