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
//#include "NES_ppu.h"
//#include "NES_apu.h"
//#include "NES_mapper.h"
//#include "NES_file.h"

CPU cpu_init() 
{
    CPU cpu =
    {
    .a = 0x00,
    .x = 0x00,
    .y = 0x00,
    .s = 0xFD, // top of the stack on reset
    .p = 0x24,
    .pc = 0xFFFC
    };
    return cpu;
}

void cpu_reset(CPU *cpu, BUS *bus)
{
    uint8_t lo = bus_read(bus, 0xFFFC);
    uint8_t hi = bus_read(bus, 0xFFFD);
    cpu->pc = (hi << 8) | lo;
}
/*
typedef struct BUS {
    uint8_t ram[0x0800];
    uint8_t *prg_banks[0x04];
    uint8_t *chr_banks[0x06];
    struct MAPPER *mapper;
    struct NESfile *rom_file;
    struct PPU *ppu;
    struct APU *apu;

}BUS;
*/
MAPPER mapper_init()
{
    MAPPER mapper = {0};
    return mapper;

}

BUS bus_init(NESfile *rom_file,PPU *ppu, APU *apu, MAPPER *mapper) 
{
    //uint8_t *ram = malloc(0x0800 * sizeof(uint8_t));
    //uint8_t ram[0x0800];
    //uint8_t *prg_banks[0x04];
    //uint8_t *chr_banks[0x06];
    //uint8_t *R = malloc(0x08 * sizeof(uint8_t));B
    //struct MAPPER *mapper = malloc(sizeof(struct MAPPER));

    //mapper->R = R;
    //mapper->bank_select = 0;
    

    
    BUS bus = 
    {
        .mapper = mapper,
        .rom_file = rom_file,
        .ppu = ppu,
        .apu = apu,
    };

    uint8_t total_prg_banks = rom_file->prg_rom_size * 2;
    bus.prg_banks[0] = rom_file->prg_rom + ((total_prg_banks - 4) * 0x2000);                         
    bus.prg_banks[1] = rom_file->prg_rom + ((total_prg_banks - 3) * 0x2000);                       
    bus.prg_banks[2] = rom_file->prg_rom + ((total_prg_banks - 2) * 0x2000);   // Second to last bank
    bus.prg_banks[3] = rom_file->prg_rom + ((total_prg_banks - 1) * 0x2000);   // Last Bank

    uint8_t total_chr_banks = rom_file->chr_rom_size * 8;
    bus.chr_banks[0] = rom_file->chr_rom + ((total_chr_banks - 6) * 0x0400); 
    bus.chr_banks[1] = rom_file->chr_rom + ((total_chr_banks - 5) * 0x0400);
    bus.chr_banks[2] = rom_file->chr_rom + ((total_chr_banks - 4) * 0x0400);
    bus.chr_banks[3] = rom_file->chr_rom + ((total_chr_banks - 3) * 0x0400);
    bus.chr_banks[4] = rom_file->chr_rom + ((total_chr_banks - 2) * 0x0400);
    bus.chr_banks[5] = rom_file->chr_rom + ((total_chr_banks - 1) * 0x0400);
    //memcpy(bus.ram, ram, 0x0800);
    return bus;
}

Instruction table[256];

void init_table() {
    // LDA
    table[0xA9] = (Instruction){ lda, addr_imm,     2};
    table[0xA5] = (Instruction){ lda, addr_zp,      3};
    table[0xB5] = (Instruction){ lda, addr_zpx,     4};
    table[0xAD] = (Instruction){ lda, addr_abs,     4};
    table[0xBD] = (Instruction){ lda, addr_absx,    4};
    table[0xB9] = (Instruction){ lda, addr_absy,    4};
    table[0xA1] = (Instruction){ lda, addr_indirx,  6};
    table[0xB1] = (Instruction){ lda, addr_indiry,  5};
    // LDX
    table[0xA2] = (Instruction){ ldx, addr_imm, 2};
    table[0xA6] = (Instruction){ ldx, addr_zp, 3};
    table[0xB6] = (Instruction){ ldx, addr_zpy, 4};
    table[0xAE] = (Instruction){ ldx, addr_abs, 4};
    table[0xBE] = (Instruction){ ldx, addr_absy, 4};
    // LDY
    table[0xA0] = (Instruction){ ldy, addr_imm, 2};
    table[0xA4] = (Instruction){ ldy, addr_zp, 3};
    table[0xB4] = (Instruction){ ldy, addr_zpx, 4};
    table[0xAC] = (Instruction){ ldy, addr_abs, 4};
    table[0xBC] = (Instruction){ ldy, addr_absx, 4};
    // STA
    table[0x85] = (Instruction){ sta, addr_zp, 3};
    table[0x95] = (Instruction){ sta, addr_zpx, 4};
    table[0x8D] = (Instruction){ sta, addr_abs, 4};
    table[0x9D] = (Instruction){ sta, addr_absx, 5};
    table[0x99] = (Instruction){ sta, addr_absy, 5};
    table[0x81] = (Instruction){ sta, addr_indirx, 6};
    table[0x91] = (Instruction){ sta, addr_indiry, 6};
    // STX
    table[0x86] = (Instruction){ stx, addr_zp, 3};
    table[0x96] = (Instruction){ stx, addr_zpy, 4};
    table[0x8E] = (Instruction){ stx, addr_abs, 4};
    // STY
    table[0x84] = (Instruction){ sty, addr_zp, 3};
    table[0x94] = (Instruction){ sty, addr_zpx, 4};
    table[0x8C] = (Instruction){ sty, addr_abs, 4};
    // TRANSFERS IMPLIED
    table[0xAA] = (Instruction){ tax, addr_imp, 2};
    table[0xA8] = (Instruction){ tay, addr_imp, 2};
    table[0xBA] = (Instruction){ tsx, addr_imp, 2};
    table[0x8A] = (Instruction){ txa, addr_imp, 2};
    table[0x9A] = (Instruction){ txs, addr_imp, 2};
    table[0x98] = (Instruction){ tya, addr_imp, 2};
    // Stack Instructions
    table[0x48] = (Instruction){ pha, addr_imp, 3};
    table[0x08] = (Instruction){ php, addr_imp, 3};
    table[0x68] = (Instruction){ pla, addr_imp, 3};
    table[0x28] = (Instruction){ plp, addr_imp, 3};
    // DEC
    table[0xC6] = (Instruction){ dec, addr_zp, 5};
    table[0xD6] = (Instruction){ dec, addr_zpx, 6};
    table[0xCE] = (Instruction){ dec, addr_abs, 6};
    table[0xDE] = (Instruction){ dec, addr_absx, 7};
    // INC
    table[0xE6] = (Instruction){ inc, addr_zp, 5};
    table[0xF6] = (Instruction){ inc, addr_zpx, 6};
    table[0xEE] = (Instruction){ inc, addr_abs, 6};
    table[0xFE] = (Instruction){ inc, addr_absx, 7};
    // DEC/INC X/Y
    table[0xCA] = (Instruction){ dex, addr_imp, 2};
    table[0x88] = (Instruction){ dey, addr_imp, 2};
    table[0xE8] = (Instruction){ inx, addr_imp, 2};
    table[0xC8] = (Instruction){ iny, addr_imp, 2};
    //ADC
    table[0x69] = (Instruction){ adc, addr_imm,     2};
    table[0x65] = (Instruction){ adc, addr_zp,      3};
    table[0x75] = (Instruction){ adc, addr_zpx,     4};
    table[0x6D] = (Instruction){ adc, addr_abs,     4};
    table[0x7F] = (Instruction){ adc, addr_absx,    4};
    table[0x79] = (Instruction){ adc, addr_absy,    4};
    table[0x61] = (Instruction){ adc, addr_indirx,  6};
    table[0x71] = (Instruction){ adc, addr_indiry,  5};
    // SBC
    table[0xE9] = (Instruction){ sbc, addr_imm, 2};
    table[0xE5] = (Instruction){ sbc, addr_zp, 3};
    table[0xF5] = (Instruction){ sbc, addr_zpx, 4};
    table[0xED] = (Instruction){ sbc, addr_abs, 4};
    table[0xFD] = (Instruction){ sbc, addr_absx, 4};
    table[0xF9] = (Instruction){ sbc, addr_absy, 4};
    table[0xE1] = (Instruction){ sbc, addr_indirx, 6};
    table[0xF1] = (Instruction){ sbc, addr_indiry, 5};
    // AND
    table[0x29] = (Instruction){ and, addr_imm, 2};
    table[0x25] = (Instruction){ and, addr_zp, 2};
    table[0x35] = (Instruction){ and, addr_zpx, 2};
    table[0x2D] = (Instruction){ and, addr_abs, 3};
    table[0x3D] = (Instruction){ and, addr_absx, 3};
    table[0x39] = (Instruction){ and, addr_absy, 3};
    table[0x21] = (Instruction){ and, addr_indirx, 2};
    table[0x31] = (Instruction){ and, addr_indiry, 2};
    // EOR
    table[0x49] = (Instruction){ eor, addr_imm, 2};
    table[0x45] = (Instruction){ eor, addr_zp, 3};
    table[0x55] = (Instruction){ eor, addr_zpx, 4};
    table[0x4D] = (Instruction){ eor, addr_abs, 4};
    table[0x5D] = (Instruction){ eor, addr_absx, 4};
    table[0x59] = (Instruction){ eor, addr_absy, 4};
    table[0x41] = (Instruction){ eor, addr_indirx, 6};
    table[0x51] = (Instruction){ eor, addr_indiry, 5};
    // ORA
    table[0x09] = (Instruction){ ora, addr_imm, 2};
    table[0x05] = (Instruction){ ora, addr_zp, 3};
    table[0x15] = (Instruction){ ora, addr_zpx, 4};
    table[0x0D] = (Instruction){ ora, addr_abs, 4};
    table[0x1D] = (Instruction){ ora, addr_absx, 4};
    table[0x19] = (Instruction){ ora, addr_absy, 4};
    table[0x01] = (Instruction){ ora, addr_indirx, 6};
    table[0x11] = (Instruction){ ora, addr_indiry, 5};
    // ASL
    table[0x0A] = (Instruction){ asl_a, addr_imp, 2};
    table[0x06] = (Instruction){ asl, addr_zp, 5};
    table[0x16] = (Instruction){ asl, addr_zpx, 6};
    table[0x0E] = (Instruction){ asl, addr_abs, 6};
    table[0x1E] = (Instruction){ asl, addr_absx, 7};
    // LSR
    table[0x4A] = (Instruction){ lsr_a, addr_imp, 2};
    table[0x46] = (Instruction){ lsr, addr_zp, 5};
    table[0x56] = (Instruction){ lsr, addr_zpx, 6};
    table[0x4E] = (Instruction){ lsr, addr_abs, 6};
    table[0x5E] = (Instruction){ lsr, addr_absx, 7};
    // ROL
    table[0x2A] = (Instruction){ rol_a, addr_imp, 2};
    table[0x26] = (Instruction){ rol, addr_zp, 5};
    table[0x36] = (Instruction){ rol, addr_zpx, 6};
    table[0x2E] = (Instruction){ rol, addr_abs, 6};
    table[0x3E] = (Instruction){ rol, addr_absx, 7};
    // ROR
    table[0x6A] = (Instruction){ ror_a, addr_imp, 2};
    table[0x66] = (Instruction){ ror, addr_zp, 5};
    table[0x76] = (Instruction){ ror, addr_zpx, 6};
    table[0x6E] = (Instruction){ ror, addr_abs, 6};
    table[0x7E] = (Instruction){ ror, addr_absx, 7};
    // SETS + CLEARS
    table[0x18] = (Instruction){ clc, addr_imp, 2};
    table[0xD8] = (Instruction){ cld, addr_imp, 2};
    table[0x58] = (Instruction){ cli, addr_imp, 2};
    table[0xB8] = (Instruction){ clv, addr_imp, 2};
    table[0x38] = (Instruction){ sec, addr_imp, 2};
    table[0xF8] = (Instruction){ sed, addr_imp, 2};
    table[0x78] = (Instruction){ sei, addr_imp, 2};
    // CMP
    table[0xC9] = (Instruction){ cmp, addr_imm, 2};
    table[0xC5] = (Instruction){ cmp, addr_zp, 3};
    table[0xD5] = (Instruction){ cmp, addr_zpx, 4};
    table[0xCD] = (Instruction){ cmp, addr_abs, 4};
    table[0xDD] = (Instruction){ cmp, addr_absx, 4};
    table[0xD9] = (Instruction){ cmp, addr_absy, 4};
    table[0xC1] = (Instruction){ cmp, addr_indirx, 6};
    table[0xD1] = (Instruction){ cmp, addr_indiry, 5};
    // CPX
    table[0xE0] = (Instruction){ cpx, addr_imm, 2};
    table[0xE4] = (Instruction){ cpx, addr_zp, 3};
    table[0xEC] = (Instruction){ cpx, addr_abs, 4};
    // CPY
    table[0xC0] = (Instruction){ cpy, addr_imm, 2};
    table[0xC4] = (Instruction){ cpy, addr_zp, 3};
    table[0xCC] = (Instruction){ cpy, addr_abs, 4};
    // BIT
    table[0x24] = (Instruction){ bit, addr_zp, 3};
    table[0x2C] = (Instruction){ bit, addr_abs, 4};
    // Branches
    table[0x90] = (Instruction){ bcc, addr_rel, 2};
    table[0xB0] = (Instruction){ bcs, addr_rel, 2};
    table[0xF0] = (Instruction){ beq, addr_rel, 2};
    table[0x30] = (Instruction){ bmi, addr_rel, 2};
    table[0xD0] = (Instruction){ bne, addr_rel, 2};
    table[0x10] = (Instruction){ bpl, addr_rel, 2};
    table[0x50] = (Instruction){ bvc, addr_rel, 2};
    table[0x70] = (Instruction){ bvs, addr_rel, 2};
    // Jumps and Returns
    table[0x4C] = (Instruction){ jmp, addr_abs, 3};
    table[0x6C] = (Instruction){ jmp, addr_indir, 5};
    table[0x20] = (Instruction){ jsr, addr_abs, 6};
    table[0x60] = (Instruction){ rts, addr_imp, 6};
    // Interrupts
    table[0x00] = (Instruction){ brk, addr_imp, 7};
    table[0x40] = (Instruction){ rti, addr_imp, 6};
    // NOP
    table[0xEA] = (Instruction){ nop, addr_imp, 2};


}

/*
Initialize Banks
void init_banks (BUS *bus)
{
    bus ->prg_banks[0];
}
    */
//CPU STEP

void cpu_step(CPU *cpu, BUS *bus)
{
    uint8_t page_crossed =0;
    cpu->opcode = bus_read(bus,cpu->pc++); // Read the current byte (op_code), prepare to read the next byte (use addressing modes)
    /*cpu->opcode_asm = disassemble(cpu->opcode).name;
    printf("OP:%02X OP_ASM:%s\n",cpu->opcode,cpu->opcode_asm);*/
    Instruction inst = table[cpu->opcode]; // find opcode in the table
    uint16_t addr = inst.addrmode(cpu, bus, &page_crossed); // address according to addressing modes 
    cpu->cycles      = inst.cycles;
    cpu->cycles     += page_crossed;
    inst.operate(cpu,bus,addr); // tick up the cycles if need

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
                bus->mapper->bank_select = data;
                break;
            case 0x8001: // bank data
                // handle bank switching
                bank_switch(bus,data);
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

void bank_switch(BUS *bus,uint8_t data)
{    
    uint8_t target      =  bus->mapper->bank_select & 0x07;
    uint8_t prg_mode    = (bus->mapper->bank_select >> 6) & 0x01;
    uint8_t chr_inv     = (bus->mapper->bank_select >> 7) & 0x01;

    bus->mapper->R[target] = data;

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
            bus->prg_banks[0] = bus->rom_file->prg_rom + (bus->mapper->R[6] * 0x2000);                            // Mapper Register 6
            bus->prg_banks[1] = bus->rom_file->prg_rom + (bus->mapper->R[7] * 0x2000);                            // Mapper Register 7
            bus->prg_banks[2] = bus->rom_file->prg_rom + (((bus->rom_file->prg_rom_size/0x2000) - 2) * 0x2000);   // Second to last bank
            bus->prg_banks[3] = bus->rom_file->prg_rom + (((bus->rom_file->prg_rom_size/0x2000) - 1) * 0x2000);   // Last Bank
        }
        else //prg_mode == 1 
        {
            bus->prg_banks[0] = bus->rom_file->prg_rom + (((bus->rom_file->prg_rom_size/0x2000) - 2) * 0x2000);   // Second to last bank
            bus->prg_banks[1] = bus->rom_file->prg_rom + (bus->mapper->R[7] * 0x2000);                            // Mapper Register 7
            bus->prg_banks[2] = bus->rom_file->prg_rom + (bus->mapper->R[6] * 0x2000);                            // Mapper Register 6
            bus->prg_banks[3] = bus->rom_file->prg_rom + (((bus->rom_file->prg_rom_size/0x2000) - 1) * 0x2000);   // Last bank
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
            bus->chr_banks[0] = bus->rom_file->chr_rom + (bus->mapper->R[0] * 0x0400); //2kB banks, but pointer goes to the start! so the R0 and R1 should ALWAYS POINT TO EVEN banks and shouldn't overlap - and if they do, thats not my fault...
            bus->chr_banks[1] = bus->rom_file->chr_rom + (bus->mapper->R[1] * 0x0400);
            bus->chr_banks[2] = bus->rom_file->chr_rom + (bus->mapper->R[2] * 0x0400);
            bus->chr_banks[3] = bus->rom_file->chr_rom + (bus->mapper->R[3] * 0x0400);
            bus->chr_banks[4] = bus->rom_file->chr_rom + (bus->mapper->R[4] * 0x0400);
            bus->chr_banks[5] = bus->rom_file->chr_rom + (bus->mapper->R[5] * 0x0400);
        }
        else //char_inv ==1
        {
            bus->chr_banks[0] = bus->rom_file->chr_rom + (bus->mapper->R[2] * 0x0400); 
            bus->chr_banks[1] = bus->rom_file->chr_rom + (bus->mapper->R[3] * 0x0400);
            bus->chr_banks[2] = bus->rom_file->chr_rom + (bus->mapper->R[4] * 0x0400);
            bus->chr_banks[3] = bus->rom_file->chr_rom + (bus->mapper->R[5] * 0x0400);
            bus->chr_banks[4] = bus->rom_file->chr_rom + (bus->mapper->R[0] * 0x0400);
            bus->chr_banks[5] = bus->rom_file->chr_rom + (bus->mapper->R[1] * 0x0400);
        }

    }
}




void disassemble(uint8_t opcode, uint16_t pc, BUS *bus, char *out, size_t out_size)
{
    uint8_t op1 = bus_read(bus, pc + 1);  // first operand byte
    uint8_t op2 = bus_read(bus, pc + 2);  // second operand byte
    uint16_t abs_addr = (op2 << 8) | op1; // little endian

    switch (opcode)
    {
        // LDA
        case 0xA9: snprintf(out, out_size, "LDA #$%02X",        op1);       break;
        case 0xA5: snprintf(out, out_size, "LDA $%02X",         op1);       break;
        case 0xB5: snprintf(out, out_size, "LDA $%02X,X",       op1);       break;
        case 0xAD: snprintf(out, out_size, "LDA $%04X",         abs_addr);  break;
        case 0xBD: snprintf(out, out_size, "LDA $%04X,X",       abs_addr);  break;
        case 0xB9: snprintf(out, out_size, "LDA $%04X,Y",       abs_addr);  break;
        case 0xA1: snprintf(out, out_size, "LDA ($%02X,X)",     op1);       break;
        case 0xB1: snprintf(out, out_size, "LDA ($%02X),Y",     op1);       break;
        // LDX
        case 0xA2: snprintf(out, out_size, "LDX #$%02X",        op1);       break;
        case 0xA6: snprintf(out, out_size, "LDX $%02X",         op1);       break;
        case 0xB6: snprintf(out, out_size, "LDX $%02X,Y",       op1);       break;
        case 0xAE: snprintf(out, out_size, "LDX $%04X",         abs_addr);  break;
        case 0xBE: snprintf(out, out_size, "LDX $%04X,Y",       abs_addr);  break;
        // LDY
        case 0xA0: snprintf(out, out_size, "LDY #$%02X",        op1);       break;
        case 0xA4: snprintf(out, out_size, "LDY $%02X",         op1);       break;
        case 0xB4: snprintf(out, out_size, "LDY $%02X,X",       op1);       break;
        case 0xAC: snprintf(out, out_size, "LDY $%04X",         abs_addr);  break;
        case 0xBC: snprintf(out, out_size, "LDY $%04X,X",       abs_addr);  break;
        // STA
        case 0x85: snprintf(out, out_size, "STA $%02X",         op1);       break;
        case 0x95: snprintf(out, out_size, "STA $%02X,X",       op1);       break;
        case 0x8D: snprintf(out, out_size, "STA $%04X",         abs_addr);  break;
        case 0x9D: snprintf(out, out_size, "STA $%04X,X",       abs_addr);  break;
        case 0x99: snprintf(out, out_size, "STA $%04X,Y",       abs_addr);  break;
        case 0x81: snprintf(out, out_size, "STA ($%02X,X)",     op1);       break;
        case 0x91: snprintf(out, out_size, "STA ($%02X),Y",     op1);       break;
        // STX
        case 0x86: snprintf(out, out_size, "STX $%02X",         op1);       break;
        case 0x96: snprintf(out, out_size, "STX $%02X,Y",       op1);       break;
        case 0x8E: snprintf(out, out_size, "STX $%04X",         abs_addr);  break;
        // STY
        case 0x84: snprintf(out, out_size, "STY $%02X",         op1);       break;
        case 0x94: snprintf(out, out_size, "STY $%02X,X",       op1);       break;
        case 0x8C: snprintf(out, out_size, "STY $%04X",         abs_addr);  break;
        // TRANSFERS
        case 0xAA: snprintf(out, out_size, "TAX");                          break;
        case 0xA8: snprintf(out, out_size, "TAY");                          break;
        case 0xBA: snprintf(out, out_size, "TSX");                          break;
        case 0x8A: snprintf(out, out_size, "TXA");                          break;
        case 0x9A: snprintf(out, out_size, "TXS");                          break;
        case 0x98: snprintf(out, out_size, "TYA");                          break;
        // STACK
        case 0x48: snprintf(out, out_size, "PHA");                          break;
        case 0x08: snprintf(out, out_size, "PHP");                          break;
        case 0x68: snprintf(out, out_size, "PLA");                          break;
        case 0x28: snprintf(out, out_size, "PLP");                          break;
        // DEC
        case 0xC6: snprintf(out, out_size, "DEC $%02X",         op1);       break;
        case 0xD6: snprintf(out, out_size, "DEC $%02X,X",       op1);       break;
        case 0xCE: snprintf(out, out_size, "DEC $%04X",         abs_addr);  break;
        case 0xDE: snprintf(out, out_size, "DEC $%04X,X",       abs_addr);  break;
        // INC
        case 0xE6: snprintf(out, out_size, "INC $%02X",         op1);       break;
        case 0xF6: snprintf(out, out_size, "INC $%02X,X",       op1);       break;
        case 0xEE: snprintf(out, out_size, "INC $%04X",         abs_addr);  break;
        case 0xFE: snprintf(out, out_size, "INC $%04X,X",       abs_addr);  break;
        // DEX/DEY/INX/INY
        case 0xCA: snprintf(out, out_size, "DEX");                          break;
        case 0x88: snprintf(out, out_size, "DEY");                          break;
        case 0xE8: snprintf(out, out_size, "INX");                          break;
        case 0xC8: snprintf(out, out_size, "INY");                          break;
        // ADC
        case 0x69: snprintf(out, out_size, "ADC #$%02X",        op1);       break;
        case 0x65: snprintf(out, out_size, "ADC $%02X",         op1);       break;
        case 0x75: snprintf(out, out_size, "ADC $%02X,X",       op1);       break;
        case 0x6D: snprintf(out, out_size, "ADC $%04X",         abs_addr);  break;
        case 0x7F: snprintf(out, out_size, "ADC $%04X,X",       abs_addr);  break;
        case 0x79: snprintf(out, out_size, "ADC $%04X,Y",       abs_addr);  break;
        case 0x61: snprintf(out, out_size, "ADC ($%02X,X)",     op1);       break;
        case 0x71: snprintf(out, out_size, "ADC ($%02X),Y",     op1);       break;
        // SBC
        case 0xE9: snprintf(out, out_size, "SBC #$%02X",        op1);       break;
        case 0xE5: snprintf(out, out_size, "SBC $%02X",         op1);       break;
        case 0xF5: snprintf(out, out_size, "SBC $%02X,X",       op1);       break;
        case 0xED: snprintf(out, out_size, "SBC $%04X",         abs_addr);  break;
        case 0xFD: snprintf(out, out_size, "SBC $%04X,X",       abs_addr);  break;
        case 0xF9: snprintf(out, out_size, "SBC $%04X,Y",       abs_addr);  break;
        case 0xE1: snprintf(out, out_size, "SBC ($%02X,X)",     op1);       break;
        case 0xF1: snprintf(out, out_size, "SBC ($%02X),Y",     op1);       break;
        // AND
        case 0x29: snprintf(out, out_size, "AND #$%02X",        op1);       break;
        case 0x25: snprintf(out, out_size, "AND $%02X",         op1);       break;
        case 0x35: snprintf(out, out_size, "AND $%02X,X",       op1);       break;
        case 0x2D: snprintf(out, out_size, "AND $%04X",         abs_addr);  break;
        case 0x3D: snprintf(out, out_size, "AND $%04X,X",       abs_addr);  break;
        case 0x39: snprintf(out, out_size, "AND $%04X,Y",       abs_addr);  break;
        case 0x21: snprintf(out, out_size, "AND ($%02X,X)",     op1);       break;
        case 0x31: snprintf(out, out_size, "AND ($%02X),Y",     op1);       break;
        // EOR
        case 0x49: snprintf(out, out_size, "EOR #$%02X",        op1);       break;
        case 0x45: snprintf(out, out_size, "EOR $%02X",         op1);       break;
        case 0x55: snprintf(out, out_size, "EOR $%02X,X",       op1);       break;
        case 0x4D: snprintf(out, out_size, "EOR $%04X",         abs_addr);  break;
        case 0x5D: snprintf(out, out_size, "EOR $%04X,X",       abs_addr);  break;
        case 0x59: snprintf(out, out_size, "EOR $%04X,Y",       abs_addr);  break;
        case 0x41: snprintf(out, out_size, "EOR ($%02X,X)",     op1);       break;
        case 0x51: snprintf(out, out_size, "EOR ($%02X),Y",     op1);       break;
        // ORA
        case 0x09: snprintf(out, out_size, "ORA #$%02X",        op1);       break;
        case 0x05: snprintf(out, out_size, "ORA $%02X",         op1);       break;
        case 0x15: snprintf(out, out_size, "ORA $%02X,X",       op1);       break;
        case 0x0D: snprintf(out, out_size, "ORA $%04X",         abs_addr);  break;
        case 0x1D: snprintf(out, out_size, "ORA $%04X,X",       abs_addr);  break;
        case 0x19: snprintf(out, out_size, "ORA $%04X,Y",       abs_addr);  break;
        case 0x01: snprintf(out, out_size, "ORA ($%02X,X)",     op1);       break;
        case 0x11: snprintf(out, out_size, "ORA ($%02X),Y",     op1);       break;
        // ASL
        case 0x0A: snprintf(out, out_size, "ASL A");                        break;
        case 0x06: snprintf(out, out_size, "ASL $%02X",         op1);       break;
        case 0x16: snprintf(out, out_size, "ASL $%02X,X",       op1);       break;
        case 0x0E: snprintf(out, out_size, "ASL $%04X",         abs_addr);  break;
        case 0x1E: snprintf(out, out_size, "ASL $%04X,X",       abs_addr);  break;
        // LSR
        case 0x4A: snprintf(out, out_size, "LSR A");                        break;
        case 0x46: snprintf(out, out_size, "LSR $%02X",         op1);       break;
        case 0x56: snprintf(out, out_size, "LSR $%02X,X",       op1);       break;
        case 0x4E: snprintf(out, out_size, "LSR $%04X",         abs_addr);  break;
        case 0x5E: snprintf(out, out_size, "LSR $%04X,X",       abs_addr);  break;
        // ROL
        case 0x2A: snprintf(out, out_size, "ROL A");                        break;
        case 0x26: snprintf(out, out_size, "ROL $%02X",         op1);       break;
        case 0x36: snprintf(out, out_size, "ROL $%02X,X",       op1);       break;
        case 0x2E: snprintf(out, out_size, "ROL $%04X",         abs_addr);  break;
        case 0x3E: snprintf(out, out_size, "ROL $%04X,X",       abs_addr);  break;
        // ROR
        case 0x6A: snprintf(out, out_size, "ROR A");                        break;
        case 0x66: snprintf(out, out_size, "ROR $%02X",         op1);       break;
        case 0x76: snprintf(out, out_size, "ROR $%02X,X",       op1);       break;
        case 0x6E: snprintf(out, out_size, "ROR $%04X",         abs_addr);  break;
        case 0x7E: snprintf(out, out_size, "ROR $%04X,X",       abs_addr);  break;
        // SETS + CLEARS
        case 0x18: snprintf(out, out_size, "CLC");                          break;
        case 0xD8: snprintf(out, out_size, "CLD");                          break;
        case 0x58: snprintf(out, out_size, "CLI");                          break;
        case 0xB8: snprintf(out, out_size, "CLV");                          break;
        case 0x38: snprintf(out, out_size, "SEC");                          break;
        case 0xF8: snprintf(out, out_size, "SED");                          break;
        case 0x78: snprintf(out, out_size, "SEI");                          break;
        // CMP
        case 0xC9: snprintf(out, out_size, "CMP #$%02X",        op1);       break;
        case 0xC5: snprintf(out, out_size, "CMP $%02X",         op1);       break;
        case 0xD5: snprintf(out, out_size, "CMP $%02X,X",       op1);       break;
        case 0xCD: snprintf(out, out_size, "CMP $%04X",         abs_addr);  break;
        case 0xDD: snprintf(out, out_size, "CMP $%04X,X",       abs_addr);  break;
        case 0xD9: snprintf(out, out_size, "CMP $%04X,Y",       abs_addr);  break;
        case 0xC1: snprintf(out, out_size, "CMP ($%02X,X)",     op1);       break;
        case 0xD1: snprintf(out, out_size, "CMP ($%02X),Y",     op1);       break;
        // CPX
        case 0xE0: snprintf(out, out_size, "CPX #$%02X",        op1);       break;
        case 0xE4: snprintf(out, out_size, "CPX $%02X",         op1);       break;
        case 0xEC: snprintf(out, out_size, "CPX $%04X",         abs_addr);  break;
        // CPY
        case 0xC0: snprintf(out, out_size, "CPY #$%02X",        op1);       break;
        case 0xC4: snprintf(out, out_size, "CPY $%02X",         op1);       break;
        case 0xCC: snprintf(out, out_size, "CPY $%04X",         abs_addr);  break;
        // BIT
        case 0x24: snprintf(out, out_size, "BIT $%02X",         op1);       break;
        case 0x2C: snprintf(out, out_size, "BIT $%04X",         abs_addr);  break;
        // BRANCHES - show target address (pc+2+offset)
        case 0x90: snprintf(out, out_size, "BCC $%04X", (uint16_t)(pc + 2 + (int8_t)op1)); break;
        case 0xB0: snprintf(out, out_size, "BCS $%04X", (uint16_t)(pc + 2 + (int8_t)op1)); break;
        case 0xF0: snprintf(out, out_size, "BEQ $%04X", (uint16_t)(pc + 2 + (int8_t)op1)); break;
        case 0x30: snprintf(out, out_size, "BMI $%04X", (uint16_t)(pc + 2 + (int8_t)op1)); break;
        case 0xD0: snprintf(out, out_size, "BNE $%04X", (uint16_t)(pc + 2 + (int8_t)op1)); break;
        case 0x10: snprintf(out, out_size, "BPL $%04X", (uint16_t)(pc + 2 + (int8_t)op1)); break;
        case 0x50: snprintf(out, out_size, "BVC $%04X", (uint16_t)(pc + 2 + (int8_t)op1)); break;
        case 0x70: snprintf(out, out_size, "BVS $%04X", (uint16_t)(pc + 2 + (int8_t)op1)); break;
        // JUMPS
        case 0x4C: snprintf(out, out_size, "JMP $%04X",         abs_addr);  break;
        case 0x6C: snprintf(out, out_size, "JMP ($%04X)",       abs_addr);  break;
        case 0x20: snprintf(out, out_size, "JSR $%04X",         abs_addr);  break;
        case 0x60: snprintf(out, out_size, "RTS");                          break;
        // INTERRUPTS
        case 0x00: snprintf(out, out_size, "BRK");                          break;
        case 0x40: snprintf(out, out_size, "RTI");                          break;
        // NOP
        case 0xEA: snprintf(out, out_size, "NOP");                          break;

        default:   snprintf(out, out_size, "??? $%02X",         opcode);    break;
    }
}