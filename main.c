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

void log_data(FILE *logfile, uint8_t opcode, uint8_t a, uint8_t x, uint8_t y, uint8_t s, uint8_t p, uint16_t pc, int cycles, BUS *bus)
{
    char disasm[64];
    disassemble(opcode, pc, bus, x, y, disasm, sizeof(disasm));
    fprintf(logfile, "%04X\t%-40s\tA:%02X\tX:%02X\tY:%02X\tP:%02X\tSP:%02X\tPPU:000,000\tCYC:%d\n",
            pc, disasm,
            a, x, y, p, s, cycles);
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
    cpu.pc      = 0xC000;
    cpu.cycles  = 0x07;
    int i = 0;
    while (i<8991)
    {
        /*if (i==5003)
        {
            printf("Hello");
        }*/
        uint16_t pc = cpu.pc;
        int cycles = cpu.cycles;
        uint8_t a   = cpu.a;
        uint8_t x   = cpu.x;
        uint8_t y   = cpu.y;
        uint8_t p   = cpu.p;
        uint8_t s   = cpu.s;
        //printf("%d\n",i);
        cpu_step(&cpu,&bus);
        log_data(logfile,cpu.opcode, a, x, y, s, p, pc, cycles, &bus);
        i++;
    }
    fclose(logfile);
    return 0;
}

