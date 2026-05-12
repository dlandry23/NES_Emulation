/*
NES_mapper.c - we  are using MMC3 - just doing for that one for now
*/
#include "NES_mapper.h"
#include "6502_cpu.h"

void bank_switch(BUS *bus,uint8_t data)
{    
    uint8_t target      =  bus->mapper.bank_select & 0x07;
    uint8_t prg_mode    = (bus->mapper.bank_select >> 6) & 0x01;
    uint8_t chr_inv     = (bus->mapper.bank_select >> 7) & 0x01;

    bus->mapper.R[target] = data;

    if (target>=6)
    {
        /*
        PRG map mode → 	   |     $8000.D6 = 0 |	$8000.D6 = 1
        -----------------------------------------------------
        CPU Bank 	       |        Value of MMC3 register
        ------------------------------------------------------
        $8000-$9FFF 	   |         R6 	 |       (-2)
        $A000-$BFFF 	   |         R7 	 |       R7
        $C000-$DFFF 	   |         (-2) 	 |       R6
        $E000-$FFFF 	   |         (-1) 	 |       (-1) 
        */
        if (prg_mode == 0)
        {
            bus->prg_banks[0] = bus->rom_file.prg_rom + (bus->mapper.R[6] * 0x2000);                            // Mapper Register 6
            bus->prg_banks[1] = bus->rom_file.prg_rom + (bus->mapper.R[7] * 0x2000);                            // Mapper Register 7
            bus->prg_banks[2] = bus->rom_file.prg_rom + (((bus->rom_file.prg_rom_size/0x2000) - 2) * 0x2000);   // Second to last bank
            bus->prg_banks[3] = bus->rom_file.prg_rom + (((bus->rom_file.prg_rom_size/0x2000) - 1) * 0x2000);   // Last Bank
        }
        else //prg_mode == 1 
        {
            bus->prg_banks[0] = bus->rom_file.prg_rom + (((bus->rom_file.prg_rom_size/0x2000) - 2) * 0x2000);   // Second to last bank
            bus->prg_banks[1] = bus->rom_file.prg_rom + (bus->mapper.R[7] * 0x2000);                            // Mapper Register 7
            bus->prg_banks[2] = bus->rom_file.prg_rom + (bus->mapper.R[6] * 0x2000);                            // Mapper Register 6
            bus->prg_banks[3] = bus->rom_file.prg_rom + (((bus->rom_file.prg_rom_size/0x2000) - 1) * 0x2000);   // Last bank
        }
    }
    else
    {
        /*
        CHR map mode → 	$8000.D7 = 0 	$8000.D7 = 1
        --------------------------------------------
        PPU Bank 	        Value of MMC3 register
        --------------------------------------------
        $0000-$03FF 	    R0 	            R2
        $0400-$07FF 	                    R3
        $0800-$0BFF 	    R1 	            R4
        $0C00-$0FFF 	                    R5
        $1000-$13FF 	    R2 	            R0
        $1400-$17FF 	    R3
        $1800-$1BFF 	    R4 	            R1 
        */
        if (chr_inv == 0)
        {
            bus->chr_banks[0] = bus->rom_file.chr_rom + (bus->mapper.R[0] * 0x0400); //2kB banks, but pointer goes to the start! so the R0 and R1 should ALWAYS POINT TO EVEN banks and shouldn't overlap - and if they do, thats not my fault...
            bus->chr_banks[1] = bus->rom_file.chr_rom + (bus->mapper.R[1] * 0x0400);
            bus->chr_banks[2] = bus->rom_file.chr_rom + (bus->mapper.R[2] * 0x0400);
            bus->chr_banks[3] = bus->rom_file.chr_rom + (bus->mapper.R[3] * 0x0400);
            bus->chr_banks[4] = bus->rom_file.chr_rom + (bus->mapper.R[4] * 0x0400);
            bus->chr_banks[5] = bus->rom_file.chr_rom + (bus->mapper.R[5] * 0x0400);
        }
        else //char_inv ==1
        {
            bus->chr_banks[0] = bus->rom_file.chr_rom + (bus->mapper.R[2] * 0x0400); 
            bus->chr_banks[1] = bus->rom_file.chr_rom + (bus->mapper.R[3] * 0x0400);
            bus->chr_banks[2] = bus->rom_file.chr_rom + (bus->mapper.R[4] * 0x0400);
            bus->chr_banks[3] = bus->rom_file.chr_rom + (bus->mapper.R[5] * 0x0400);
            bus->chr_banks[4] = bus->rom_file.chr_rom + (bus->mapper.R[0] * 0x0400);
            bus->chr_banks[5] = bus->rom_file.chr_rom + (bus->mapper.R[1] * 0x0400);
        }

    }
    //TODO - Write this thang
    printf('NOT IMPLEMENTED YET');
}


