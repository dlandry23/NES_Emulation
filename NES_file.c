/*
NES_file.h
*/
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "NES_file.h"

// .nes file
void NESfile_init(NESfile *rom_file)
{
    //rom_file->filename =  filename;
    //Initialize the file;
    FILE *f= fopen(rom_file->filename, "rb");
    if (!f) return;
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
        fclose(f);return;
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
        fclose(f);return;
    }

    if (fread(rom_file->chr_rom,1,chr_bytes,f) !=chr_bytes) // read the chr_rom
    {
        fclose(f);return;
    }

    fclose(f);
    return;
}