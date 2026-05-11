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

// .nes file
void NESfile_init(NESfile *rom_file)
{
    //Initialize the file;
    FILE *f= fopen(rom_file->filename, "rb");
    if (!f) return -1;
    /*if (f==NULL)
	{
		printf("error: Couldn't open %s\n", rom_file->filename);
		exit(1);
	}*/
    //get file size -- DONT NEED THIS - Know header, know rest of file...
    /*
    fseek(f,0L,SEEK_END);
    int fsize = ftell(f);
    fseek(f,0L,SEEK_SET);
    
    uint8_t *buffer_rom = (uint8_t*)malloc(fsize);//allocate memory (fsize) -> use to read entire rom
    */

    if (fread(rom_file->header,1,0x10,f) !=0x10) // read the header - check if we got all 16 bytes, if yes, cool
    {
        fclose(f);return -1;
    }

    
    rom_file->prg_rom_size = rom_file->header[4];// in 16KB units
    rom_file->chr_rom_size = rom_file->header[5];
    rom_file->mapper       = (rom_file->header[6] >> 4) | (rom_file->header[7] & 0xF0);

    size_t prg_bytes = rom_file->prg_rom_size*0x4000; // 16KB sections
    rom_file->prg_rom = malloc(prg_bytes);
    size_t chr_bytes = rom_file->chr_rom_size*0x2000; // 8KB sections
    rom_file->chr_rom = malloc(chr_bytes);

    if (fread(rom_file->prg_rom,1,prg_bytes,f) !=prg_bytes) // read the prg_rom
    {
        fclose(f);return -1;
    }

    if (fread(rom_file->chr_rom,1,chr_bytes,f) !=chr_bytes) // read the chr_rom
    {
        fclose(f);return -1;
    }

    fclose(f);
    return 0;




    /*//uint8_t *buffer = &state->memory[offset]; //Reference the memory location of state as buffer, fread into the "buffer" - use to read what is in the memory banks (particularly just last bank matters)
    fread(buffer_rom,1,fsize,f);
    fclose(f);
    uint8_t prg_bnk_num = buffer_rom[4];
    uint8_t chr_bnk_num = buffer_rom[5];
    memcpy(&state->memory[offset],&buffer_rom[(0x4000*(prg_bnk_num-0x4))+0x10],0x10000);
    //return buffer_rom;*/
}

//CPU STEP

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
                bank_switch(bus->prg_banks,bus->chr_banks,bus->mapper.bank_select,data);
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

