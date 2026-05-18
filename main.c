/*
main.c
*/

#include <stdint.h>
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

int main()
{
    //NESfile rom = {0};
    char *filename = "C:\\Users\\Lan\\Downloads\\SMB3rom.nes";
    printf("Hello Worl");
    NESfile rom_file = NESfile_init(filename);
    CPU cpu = cpu_init();
    PPU ppu = ppu_init();
    APU apu = apu_init();
    BUS bus = bus_init(rom_file,ppu,apu);

    while 1
    {
        cpu_step(cpu,bus)
    }

    return 0;
}

