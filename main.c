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
//#include "NES_mapper.h"
#include "NES_file.h"

void log_data(FILE *logfile, CPU *cpu, BUS *bus, uint16_t pc)
{
    char disasm[32];
    disassemble(cpu->opcode, pc, bus, disasm, sizeof(disasm));

    fprintf(logfile, "%04X  %-20s  A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%d\n",
            pc, disasm,
            cpu->a, cpu->x, cpu->y, cpu->p, cpu->s, cpu->cycles);
}

int main()
{
    FILE *logfile = fopen("nes.log","w");
    //NESfile rom = {0};
    char *filename = "C:\\Users\\Lan\\Downloads\\nestest.nes";
    //printf("Hello Worl");
    NESfile    rom_file = NESfile_init(filename);
    CPU        cpu      = cpu_init();
    PPU        ppu      = ppu_init();
    APU        apu      = apu_init();
    MAPPER     mapper   = mapper_init();
    BUS        bus      = bus_init(&rom_file,&ppu,&apu,&mapper);
    init_table();
    //cpu_reset(&cpu, &bus);
    // NES TEST START AT PC = $C000
    cpu.pc = 0xC000;
    int i = 0;
    while (i<1000)
    {
        uint16_t pc = cpu.pc;
        cpu_step(&cpu,&bus);
        log_data(logfile, &cpu, &bus, pc);
        i++;
    }
    fclose(logfile);
    return 0;
}

