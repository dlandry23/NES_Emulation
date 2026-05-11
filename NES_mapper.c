/*
NES_mapper.c - we  are using MMC3 - just doing for that one for now
*/
#include "NES_mapper.h"
void bank_switch(BUS *bus,uint8_t bank_select,uint8_t data)
{
    uint8_t target      =  bank_select & 0x07;
    uint8_t prg_mode    = (bank_select >> 6) & 0x01;
    uint8_t chr_inv     = (bank_select >> 7) & 0x01;

    bus->mapper.R[target] = data;

    if (target>=6)
    {
        if (prg_mode == 0)
        {
            bus->prg_banks[0] = bus->rom_file.prg_rom + (bus->mapper.R[6] * 0x2000);
            bus->prg_banks[1] = bus->rom_file.prg_rom + (bus->mapper.R[7] * 0x2000);
            bus->prg_banks[2] = bus->rom_file.prg_rom + (((bus->rom_file.prg_rom_size/0x2000) - 2) * 0x2000);// num_banks - set as defined somewhere? maybe put i
            bus->prg_banks[3] = bus->rom_file.prg_rom + (((bus->rom_file.prg_rom_size/0x2000) - 1) * 0x2000);
        }
        else //prg_mode == 1 
        {

        }
    }
    else
    {

    }
    //TODO - Write this thang
    printf('NOT IMPLEMENTED YET');
}


