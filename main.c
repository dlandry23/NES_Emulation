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
    NESfile rom = {0};
    //rom.filename = "C:\\Users\\Lan\\Downloads\\SMB3rom.nes";
    printf("Hello Worl");
    NESfile_init(&rom);
    return 0;
}

