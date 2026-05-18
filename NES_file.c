/*
NES_file.h
*/
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "NES_file.h"

// .nes file
NESfile NESfile_init(char *filename)
{
    //rom_file->filename =  filename;
 
        
    //Initialize the file;
    NESfile empty = {0};
    FILE *f= fopen(filename, "rb");
    if (!f) return empty;
    uint8_t header[0x10];
    if (fread(header,1,0x10,f) !=0x10) // read the header - check if we got all 16 bytes, if yes, cool
    {
        fclose(f);return empty;
    }

    
    uint8_t prg_rom_size = header[4];// in 16KB units
    uint8_t chr_rom_size = header[5];
    uint8_t mapper       = (header[6] >> 4) | (header[7] & 0xF0);

    size_t prg_bytes = prg_rom_size*0x4000; // 16KB sections
    uint8_t *prg_rom = malloc(prg_bytes);
    size_t chr_bytes = chr_rom_size*0x2000; // 8KB sections
    uint8_t *chr_rom = chr_rom_size > 0 ? malloc(chr_bytes) : NULL;

    if (fread(prg_rom,1,prg_bytes,f) !=prg_bytes) // read the prg_rom
    {
        fclose(f);return empty;
    }

    if (fread(chr_rom,1,chr_bytes,f) !=chr_bytes) // read the chr_rom
    {
        fclose(f);return empty;
    }

    fclose(f);
    NESfile nesfile =  {
        .filename = filename,
        .prg_rom = prg_rom,
        .chr_rom = chr_rom,
        .prg_rom_size = prg_rom_size, // in 16KB units
        .chr_rom_size = chr_rom_size,// in 8KB units
        .mapper = mapper // not using for now - just for future if I implement a wrapper outside of MMC3
        };
    memcpy(nesfile.header, header, 0x10);
    return nesfile;
}