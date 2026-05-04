//Rewriting 6502 Dissassembler from scratch utilizing chip reference guide
//https://www.nesdev.org/wiki/Instruction_reference

// utilize example rom (Super Mario Bros. 3 for NES), and read it in - print out instructions list - maybe write to file for ease of reference
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

//Type Definition (Setting processor state - organizing registers) 
typedef struct State6502
{
    //-----REGISTERS-----
    uint16_t    PC; // Program Counter
    uint8_t     A;  // Accumulator
    uint8_t     X;  // X Register (hold counters, offsets for accessing memory, copy stack pointer)
    uint8_t     Y;  // Y Register (no special function - similar to X)
    uint8_t     S; // Stack Pointer - it indexes into a 256 byte stack at $0100-$01FF
    uint8_t     P; // Status Register - Detailed blow
    /*
        N V 1 B D I Z C
        N - Negative Flag
        V - Overflow Flag
        1 - Always pushed as 1 (no effect)
        B - Break flag (no CPU effect)
        D - Decimal Flag
        I - Interrupt Disable Flag
        Z - Zero Flag
        C - Carry Fgag
    */
   //-----MEMORY ALLOCATION -----
   uint8_t      *memory;
   //-----CONDITION CODES?-----
   //-----ADDITIONAL FLAGS-----
}State6502;


// Functions
static int Disassemble_6502(unsigned char *codebuffer,int pc)
{
    int count=1;
    int cycles =2;
    char opstr[256];
    uint8_t *opcodes = &codebuffer[pc];
    switch (opcodes[0])
    {
        //ADC
        case 0x69: sprintf(opstr,"ADC #$%02x",opcodes[1]);count=2;cycles=2;break; //Immediate
        case 0x65: sprintf(opstr,"ADC $%02x",opcodes[1]);count=2;cycles=2;break; //Zero Page
        case 0x75: sprintf(opstr,"ADC #$%02x",opcodes[1]);count=2;cycles=2;break; //Immediate

        
        case 0x00: sprintf(opstr,"BRK");break;

        default:printf("Unaccounted opcode - moving on...");break;
    }
    printf("%04x %02x", 0x0000+pc,opcodes[0]);
    if (count > 1)
		printf("%02X ", opcodes[1]);
	else
		printf("   ");
	if (count > 2)
		printf("%02X ", opcodes[2]);
	else
		printf("   ");
	printf("%-s Cycles:%d", opstr,cycles);
    //Branches and branch targets
    switch(opcodes[0])
    {
        case 0x90:
        case 0xB0:
        case 0xF0:
        case 0x30:
        case 0xD0:
        case 0x10:
        case 0x50:
        case 0x70:
            printf("\t\t;$%04x", 0x5000+pc+2+(int8_t)opcodes[1]); // Print branch target
    }
    return count;
}

static int Emulate_6502(State6502 state, unsigned char *codebuffer)
{
    int pc = 0;
    uint8_t *opcodes = &codebuffer[pc];
    switch (opcodes[0])
    {
        //ADC
        case 0x69://Immediate
             
        case 0x65:

        default:printf("No code found - throw error");break;
    }
    int count = 1;
    return count;
}

State6502* Init_6502(void)
{
    State6502* state = calloc(1,sizeof(State6502));
    state->memory = malloc(0x10000); // This should be the size of the BANK memory of the CPU! can only see 64kB - PRG rom is 256, CHR rom is 128 (in this particular case)
    memset(state->memory,0,0x10000);
    return state;
}

uint8_t ReadFileIntoMemoryAt(State6502* state, char* filename, uint32_t offset)
{
    //Initialize the file;
    FILE *f= fopen(filename, "rb");
    if (f==NULL)
	{
		printf("error: Couldn't open %s\n", filename);
		exit(1);
	}
    //get file size
    fseek(f,0L,SEEK_END);
    int fsize = ftell(f);
    fseek(f,0L,SEEK_SET);

    uint8_t *buffer_rom = (uint8_t*)malloc(fsize);//allocate memory (fsize) -> use to read entire rom
    
    //uint8_t *buffer = &state->memory[offset]; //Reference the memory location of state as buffer, fread into the "buffer" - use to read what is in the memory banks (particularly just last bank matters)
    fread(buffer_rom,1,fsize,f);
    fclose(f);
    uint8_t prg_bnk_num = buffer_rom[4];
    uint8_t chr_bnk_num = buffer_rom[5];
    memcpy(&state->memory[offset],&buffer_rom[(0x4000*(prg_bnk_num-0x4))+0x10],0x10000);
    //return buffer_rom;
}
//Classes

//MAIN
int main()
{
    State6502* state = Init_6502(); //Since I'm not sure how much memory the rom needs, maybe dont initialize memory call first? OR make it larger than any NES rom so it covers everything
    ReadFileIntoMemoryAt(state,"C:/Users/Lan/Downloads/SMB3rom.nes",0);

    //ROM memory Allocation
    /*
        Memory Allocation in the ROM .nes files is the following:
        Typically 16byte header -> 
            0-3 	Constant $4E $45 $53 $1A (ASCII "NES" followed by MS-DOS end-of-file)
            4 	Size of PRG ROM in 16 KB units
            5 	Size of CHR ROM in 8 KB units (value 0 means the board uses CHR RAM)
            6 	Flags 6 – Mapper, mirroring, battery, trainer
            7 	Flags 7 – Mapper, VS/Playchoice, NES 2.0
            8 	Flags 8 – PRG-RAM size (rarely used extension)
            9 	Flags 9 – TV system (rarely used extension)
            10 	Flags 10 – TV system, PRG-RAM presence (unofficial, rarely used extension)
            11-15 	Unused padding (should be filled with zero, but some rippers put their name across bytes 7-15)

        FLAG 6
        76543210
        ||||||||
        |||||||+- Nametable arrangement: 0: vertical arrangement ("horizontal mirrored") (CIRAM A10 = PPU A11)
        |||||||                          1: horizontal arrangement ("vertically mirrored") (CIRAM A10 = PPU A10)
        ||||||+-- 1: Cartridge contains battery-backed PRG RAM ($6000-7FFF) or other persistent memory
        |||||+--- 1: 512-byte trainer at $7000-$71FF (stored before PRG data)
        ||||+---- 1: Alternative nametable layout
        ++++----- Lower nybble of mapper number
    */
    //When starting up - to run emulator, read first header information, then can skip to ROM data
    //Typically NES JMPs direct to starting memory address on boot
    /*
    int pc = 0;
    while (pc<fsize)
    {
        printf("$%02x ", buffer[pc], pc);
        if (buffer[pc]==0xff)
        {
            printf("Hello");
        }
        //printf(buffer[pc]);
        pc+=Disassemble_6502(buffer,pc);
    }

    while (done ==0)
    {
        done = Emulate_6502(state);
    }
    */
    return 0;
}