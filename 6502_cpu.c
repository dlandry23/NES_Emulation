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
#include "NES_ppu.h"
#include "NES_apu.h"
#include "NES_mapper.h"
#include "NES_file.h"

CPU cpu_init() 
{
    CPU cpu =
    {
    .a = 0x00,
    .x = 0x00,
    .y = 0x00,
    .s = 0xFD, // top of the stack on reset
    .p = 0x24,
    .pc = 0x0000
    };
    return cpu;
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
BUS bus_init(NESfile *rom_file,PPU *ppu, APU *apu) 
{
    uint8_t ram[0x0800];
    uint8_t *prg_banks[0x04];
    uint8_t *chr_banks[0x06];
    uint8_t R[0x08];
    struct MAPPER mapper = 
    {
        .R = R,
        .bank_select = 0,

    };
    prg_banks[0] = rom_file->prg_rom + (((rom_file->prg_rom_size/0x2000) - 4) * 0x2000);                         
    prg_banks[1] = rom_file->prg_rom + (((rom_file->prg_rom_size/0x2000) - 3) * 0x2000);                       
    prg_banks[2] = rom_file->prg_rom + (((rom_file->prg_rom_size/0x2000) - 2) * 0x2000);   // Second to last bank
    prg_banks[3] = rom_file->prg_rom + (((rom_file->prg_rom_size/0x2000) - 1) * 0x2000);   // Last Bank
    BUS bus = 
    {
        .ram = ram,
        .prg_banks = prg_banks,
        .chr_banks = chr_banks,
        .mapper = mapper,
        .rom_file = rom_file,
        .ppu = ppu,
        .apu = apu,
    };
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

//Initialize Banks
void init_banks (BUS *bus)
{
    bus ->prg_banks[0];
}
//CPU STEP

void cpu_step(CPU *cpu, BUS *bus)
{
    uint8_t page_crossed =0;
    uint8_t opcode = bus_read(bus,cpu->pc++); // Read the current byte (op_code), prepare to read the next byte (use addressing modes)
    Instruction inst = table[opcode]; // find opcode in the table
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

