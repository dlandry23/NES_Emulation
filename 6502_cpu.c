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
#include <stdbool.h>

#include "6502_cpu.h"
#include "6502_opcodes.h"
#include "6502_addrmodes.h"
//#include "NES_ppu.h"
//#include "NES_apu.h"
//#include "NES_mapper.h"
//#include "NES_file.h"

bool contains(uint8_t arr[], int size, uint8_t target) {
    for (int i = 0; i < size; i++) {
        if (arr[i] == target) {
            return true;
        }
    }
    return false;
}


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
    table[0x68] = (Instruction){ pla, addr_imp, 4};
    table[0x28] = (Instruction){ plp, addr_imp, 4};
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
    table[0x7D] = (Instruction){ adc, addr_absx,    4};
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
    table[0x25] = (Instruction){ and, addr_zp, 3};
    table[0x35] = (Instruction){ and, addr_zpx, 4};
    table[0x2D] = (Instruction){ and, addr_abs, 4};
    table[0x3D] = (Instruction){ and, addr_absx, 4};
    table[0x39] = (Instruction){ and, addr_absy, 4};
    table[0x21] = (Instruction){ and, addr_indirx, 6};
    table[0x31] = (Instruction){ and, addr_indiry, 5};
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
    // ILLEGAL OPCODES
    // NOPS
    table[0x1A] = (Instruction){ nop, addr_imp, 2};
    table[0x3A] = (Instruction){ nop, addr_imp, 2};
    table[0x5A] = (Instruction){ nop, addr_imp, 2};
    table[0x7A] = (Instruction){ nop, addr_imp, 2};
    table[0xDA] = (Instruction){ nop, addr_imp, 2};
    table[0xFA] = (Instruction){ nop, addr_imp, 2};
    table[0x80] = (Instruction){ nop, addr_imm, 2};
    table[0x82] = (Instruction){ nop, addr_imm, 2};
    table[0x89] = (Instruction){ nop, addr_imm, 2};
    table[0xC2] = (Instruction){ nop, addr_imm, 2};
    table[0xE2] = (Instruction){ nop, addr_imm, 2};
    table[0x04] = (Instruction){ nop, addr_zp, 3};
    table[0x44] = (Instruction){ nop, addr_zp, 3};
    table[0x64] = (Instruction){ nop, addr_zp, 3};
    table[0x14] = (Instruction){ nop, addr_zpx, 4};
    table[0x34] = (Instruction){ nop, addr_zpx, 4};
    table[0x54] = (Instruction){ nop, addr_zpx, 4};
    table[0x74] = (Instruction){ nop, addr_zpx, 4};
    table[0xD4] = (Instruction){ nop, addr_zpx, 4};
    table[0xF4] = (Instruction){ nop, addr_zpx, 4};
    table[0x0C] = (Instruction){ nop, addr_abs, 4};
    table[0x1C] = (Instruction){ nop, addr_absx, 4};
    table[0x3C] = (Instruction){ nop, addr_absx, 4};
    table[0x5C] = (Instruction){ nop, addr_absx, 4};
    table[0x7C] = (Instruction){ nop, addr_absx, 4};
    table[0xDC] = (Instruction){ nop, addr_absx, 4};
    table[0xFC] = (Instruction){ nop, addr_absx, 4};
    // ILLEGAL OPCODES
    table[0x4B] = (Instruction){ alr, addr_imm, 2};
    table[0x0B] = (Instruction){ anc, addr_imm, 2};
    table[0x2B] = (Instruction){ anc2, addr_imm, 2};
    //table[0x8B] = (Instruction){ ane, addr_imm, 2};
    //table[0x6B] = (Instruction){ arr, addr_imm, 2};
    table[0xC7] = (Instruction){ dcp, addr_zp, 5};
    table[0xD7] = (Instruction){ dcp, addr_zpx, 6};
    table[0xCF] = (Instruction){ dcp, addr_abs, 6};
    table[0xDF] = (Instruction){ dcp, addr_absx, 7};
    table[0xDB] = (Instruction){ dcp, addr_absy, 7};
    table[0xC3] = (Instruction){ dcp, addr_indirx, 8};
    table[0xD3] = (Instruction){ dcp, addr_indiry, 8};
    table[0xE7] = (Instruction){ isb, addr_zp, 5};
    table[0xF7] = (Instruction){ isb, addr_zpx, 6};
    table[0xEF] = (Instruction){ isb, addr_abs, 6};
    table[0xFF] = (Instruction){ isb, addr_absx, 7};
    table[0xFB] = (Instruction){ isb, addr_absy, 7};
    table[0xE3] = (Instruction){ isb, addr_indirx, 8};
    table[0xF3] = (Instruction){ isb, addr_indiry, 8};
    //table[0xBB] = (Instruction){ las, addr_absy, 4};
    table[0xA7] = (Instruction){ lax, addr_zp, 3};
    table[0xB7] = (Instruction){ lax, addr_zpy, 4};
    table[0xAF] = (Instruction){ lax, addr_abs, 4};
    table[0xBF] = (Instruction){ lax, addr_absy, 4};
    table[0xA3] = (Instruction){ lax, addr_indirx, 6};
    table[0xB3] = (Instruction){ lax, addr_indiry, 5};
    //table[0xAB] = (Instruction){ lxa, addr_imm, 2};
    table[0x27] = (Instruction){ rla, addr_zp, 5};
    table[0x37] = (Instruction){ rla, addr_zpx, 6};
    table[0x2F] = (Instruction){ rla, addr_abs, 6};
    table[0x3F] = (Instruction){ rla, addr_absx, 7};
    table[0x3B] = (Instruction){ rla, addr_absy, 7};
    table[0x23] = (Instruction){ rla, addr_indirx, 8};
    table[0x33] = (Instruction){ rla, addr_indiry, 8};
    table[0x67] = (Instruction){ rra, addr_zp, 5};
    table[0x77] = (Instruction){ rra, addr_zpx, 6};
    table[0x6F] = (Instruction){ rra, addr_abs, 6};
    table[0x7F] = (Instruction){ rra, addr_absx, 7};
    table[0x7B] = (Instruction){ rra, addr_absy, 7};
    table[0x63] = (Instruction){ rra, addr_indirx, 8};
    table[0x73] = (Instruction){ rra, addr_indiry, 8};
    table[0x87] = (Instruction){ sax, addr_zp, 3};
    table[0x97] = (Instruction){ sax, addr_zpy, 4};
    table[0x8F] = (Instruction){ sax, addr_abs, 4};
    table[0x83] = (Instruction){ sax, addr_indirx, 6};
    //table[0xCB] = (Instruction){ sbx, addr_imm, 2};
    //table[0x9F] = (Instruction){ sha, addr_absy, 5};
    //table[0x93] = (Instruction){ sha, addr_indiry, 6};
    //table[0x9E] = (Instruction){ shx, addr_absy, 5};
    //table[0x9C] = (Instruction){ shy, addr_absx, 5};
    table[0x07] = (Instruction){ slo, addr_zp, 5};
    table[0x17] = (Instruction){ slo, addr_zpx, 6};
    table[0x0F] = (Instruction){ slo, addr_abs, 6};
    table[0x1F] = (Instruction){ slo, addr_absx, 7};
    table[0x1B] = (Instruction){ slo, addr_absy, 7};
    table[0x03] = (Instruction){ slo, addr_indirx, 8};
    table[0x13] = (Instruction){ slo, addr_indiry, 8};
    table[0x47] = (Instruction){ sre, addr_zp, 5};
    table[0x57] = (Instruction){ sre, addr_zpx, 6};
    table[0x4F] = (Instruction){ sre, addr_abs, 6};
    table[0x5F] = (Instruction){ sre, addr_absx, 7};
    table[0x5B] = (Instruction){ sre, addr_absy, 7};
    table[0x43] = (Instruction){ sre, addr_indirx, 8};
    table[0x53] = (Instruction){ sre, addr_indiry, 8};
    //table[0x9B] = (Instruction){ tas, addr_absy, 5};
    table[0xEB] = (Instruction){ sbc, addr_imm, 2};




}

/*
Initialize Banks
void init_banks (BUS *bus)
{
    bus ->prg_banks[0];
}
    */
//CPU STEP
uint8_t no_page_cross[] = {0x9D,0x99,0xDF,0xDB,0xD3,0xFF,0xFB,0xF3,0X7F,0x7B,0x73,0x3F,0x5F,0x5B,0x53,0x3B,0x33,0x1F,0x1B,0x13};
int npc_size = sizeof(no_page_cross);
void cpu_step(CPU *cpu, BUS *bus)
{
    uint8_t page_crossed =0;
    cpu->opcode = bus_read(bus,cpu->pc++); // Read the current byte (op_code), prepare to read the next byte (use addressing modes)
    /*cpu->opcode_asm = disassemble(cpu->opcode).name;
    printf("OP:%02X OP_ASM:%s\n",cpu->opcode,cpu->opcode_asm);*/
    Instruction inst = table[cpu->opcode]; // find opcode   
    //printf("%02X\n",cpu->opcode);
    uint16_t addr = inst.addrmode(cpu, bus, &page_crossed); // address according to addressing modes 

    if (contains(no_page_cross,npc_size,cpu->opcode)) page_crossed = 0;
    cpu->cycles     += inst.cycles;
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




void disassemble(uint8_t opcode, uint16_t pc, BUS *bus, uint8_t x, uint8_t y, char *out, size_t out_size)
{
    uint8_t  op1      = bus_read(bus, pc + 1);
    uint8_t  op2      = bus_read(bus, pc + 2);
    uint16_t abs_addr = (op2 << 8) | op1;

    switch (opcode)
    {
        // LDA
        case 0xA9: snprintf(out, out_size, "%02X %02X        LDA #$%02X",                    opcode, op1, op1);        break;
        case 0xA5:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        LDA $%02X = %02X",                     opcode, op1, op1, val);   break;
        }
        case 0xB5:
        {
            uint8_t  eff = (op1 + x) & 0xFF;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        LDA $%02X,X @ %02X = %02X",            opcode, op1, op1, eff, val); break;
        }
        case 0xAD:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   LDA $%04X = %02X",                     opcode, op1, op2, abs_addr, val); break;
        }
        case 0xBD:
        {
            uint16_t eff = abs_addr + x;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   LDA $%04X,X @ %04X = %02X",            opcode, op1, op2, abs_addr, eff, val); break;
        }
        case 0xB9:
        {
            uint16_t eff = abs_addr + y;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   LDA $%04X,Y @ %04X = %02X",            opcode, op1, op2, abs_addr, eff, val); break;
        }
        case 0xA1:
        {
            uint8_t  zp  = (op1 + x) & 0xFF;
            uint16_t eff = (bus_read(bus, (zp + 1) & 0xFF) << 8) | bus_read(bus, zp);
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        LDA ($%02X,X) @ %02X = %04X = %02X",   opcode, op1, op1, zp, eff, val); break;
        }
        case 0xB1:
        {
            uint16_t base = (bus_read(bus, (op1 + 1) & 0xFF) << 8) | bus_read(bus, op1);
            uint16_t eff  = base + y;
            uint8_t  val  = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        LDA ($%02X),Y = %04X @ %04X = %02X",   opcode, op1, op1, base, eff, val); break;
        }
        // LDX
        case 0xA2: snprintf(out, out_size, "%02X %02X        LDX #$%02X",                    opcode, op1, op1);        break;
        case 0xA6:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        LDX $%02X = %02X",                     opcode, op1, op1, val);   break;
        }
        case 0xB6:
        {
            uint8_t  eff = (op1 + y) & 0xFF;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        LDX $%02X,Y @ %02X = %02X",            opcode, op1, op1, eff, val); break;
        }
        case 0xAE:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   LDX $%04X = %02X",                     opcode, op1, op2, abs_addr, val); break;
        }
        case 0xBE:
        {
            uint16_t eff = abs_addr + y;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   LDX $%04X,Y @ %04X = %02X",            opcode, op1, op2, abs_addr, eff, val); break;
        }
        // LDY
        case 0xA0: snprintf(out, out_size, "%02X %02X        LDY #$%02X",                    opcode, op1, op1);        break;
        case 0xA4:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        LDY $%02X = %02X",                     opcode, op1, op1, val);   break;
        }
        case 0xB4:
        {
            uint8_t  eff = (op1 + x) & 0xFF;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        LDY $%02X,X @ %02X = %02X",            opcode, op1, op1, eff, val); break;
        }
        case 0xAC:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   LDY $%04X = %02X",                     opcode, op1, op2, abs_addr, val); break;
        }
        case 0xBC:
        {
            uint16_t eff = abs_addr + x;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   LDY $%04X,X @ %04X = %02X",            opcode, op1, op2, abs_addr, eff, val); break;
        }
        // STA
        case 0x85:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        STA $%02X = %02X",                     opcode, op1, op1, val);   break;
        }
        case 0x95:
        {
            uint8_t  eff = (op1 + x) & 0xFF;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        STA $%02X,X @ %02X = %02X",            opcode, op1, op1, eff, val); break;
        }
        case 0x8D:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   STA $%04X = %02X",                     opcode, op1, op2, abs_addr, val); break;
        }
        case 0x9D:
        {
            uint16_t eff = abs_addr + x;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   STA $%04X,X @ %04X = %02X",            opcode, op1, op2, abs_addr, eff, val); break;
        }
        case 0x99:
        {
            uint16_t eff = abs_addr + y;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   STA $%04X,Y @ %04X = %02X",            opcode, op1, op2, abs_addr, eff, val); break;
        }
        case 0x81:
        {
            uint8_t  zp  = (op1 + x) & 0xFF;
            uint16_t eff = (bus_read(bus, (zp + 1) & 0xFF) << 8) | bus_read(bus, zp);
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        STA ($%02X,X) @ %02X = %04X = %02X",   opcode, op1, op1, zp, eff, val); break;
        }
        case 0x91:
        {
            uint16_t base = (bus_read(bus, (op1 + 1) & 0xFF) << 8) | bus_read(bus, op1);
            uint16_t eff  = base + y;
            uint8_t  val  = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        STA ($%02X),Y = %04X @ %04X = %02X",   opcode, op1, op1, base, eff, val); break;
        }
        // STX
        case 0x86:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        STX $%02X = %02X",                     opcode, op1, op1, val);   break;
        }
        case 0x96:
        {
            uint8_t  eff = (op1 + y) & 0xFF;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        STX $%02X,Y @ %02X = %02X",            opcode, op1, op1, eff, val); break;
        }
        case 0x8E:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   STX $%04X = %02X",                     opcode, op1, op2, abs_addr, val); break;
        }
        // STY
        case 0x84:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        STY $%02X = %02X",                     opcode, op1, op1, val);   break;
        }
        case 0x94:
        {
            uint8_t  eff = (op1 + x) & 0xFF;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        STY $%02X,X @ %02X = %02X",            opcode, op1, op1, eff, val); break;
        }
        case 0x8C:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   STY $%04X = %02X",                     opcode, op1, op2, abs_addr, val); break;
        }
        // TRANSFERS
        case 0xAA: snprintf(out, out_size, "%02X             TAX",               opcode);                       break;
        case 0xA8: snprintf(out, out_size, "%02X             TAY",               opcode);                       break;
        case 0xBA: snprintf(out, out_size, "%02X             TSX",               opcode);                       break;
        case 0x8A: snprintf(out, out_size, "%02X             TXA",               opcode);                       break;
        case 0x9A: snprintf(out, out_size, "%02X             TXS",               opcode);                       break;
        case 0x98: snprintf(out, out_size, "%02X             TYA",               opcode);                       break;
        // STACK
        case 0x48: snprintf(out, out_size, "%02X             PHA",               opcode);                       break;
        case 0x08: snprintf(out, out_size, "%02X             PHP",               opcode);                       break;
        case 0x68: snprintf(out, out_size, "%02X             PLA",               opcode);                       break;
        case 0x28: snprintf(out, out_size, "%02X             PLP",               opcode);                       break;
        // DEC
        case 0xC6:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        DEC $%02X = %02X",                     opcode, op1, op1, val);   break;
        }
        case 0xD6:
        {
            uint8_t  eff = (op1 + x) & 0xFF;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        DEC $%02X,X @ %02X = %02X",            opcode, op1, op1, eff, val); break;
        }
        case 0xCE:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   DEC $%04X = %02X",                     opcode, op1, op2, abs_addr, val); break;
        }
        case 0xDE:
        {
            uint16_t eff = abs_addr + x;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   DEC $%04X,X @ %04X = %02X",            opcode, op1, op2, abs_addr, eff, val); break;
        }
        // INC
        case 0xE6:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        INC $%02X = %02X",                     opcode, op1, op1, val);   break;
        }
        case 0xF6:
        {
            uint8_t  eff = (op1 + x) & 0xFF;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        INC $%02X,X @ %02X = %02X",            opcode, op1, op1, eff, val); break;
        }
        case 0xEE:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   INC $%04X = %02X",                     opcode, op1, op2, abs_addr, val); break;
        }
        case 0xFE:
        {
            uint16_t eff = abs_addr + x;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   INC $%04X,X @ %04X = %02X",            opcode, op1, op2, abs_addr, eff, val); break;
        }
        // DEX/DEY/INX/INY
        case 0xCA: snprintf(out, out_size, "%02X             DEX",               opcode);                       break;
        case 0x88: snprintf(out, out_size, "%02X             DEY",               opcode);                       break;
        case 0xE8: snprintf(out, out_size, "%02X             INX",               opcode);                       break;
        case 0xC8: snprintf(out, out_size, "%02X             INY",               opcode);                       break;
        // ADC
        case 0x69: snprintf(out, out_size, "%02X %02X        ADC #$%02X",                    opcode, op1, op1);        break;
        case 0x65:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        ADC $%02X = %02X",                     opcode, op1, op1, val);   break;
        }
        case 0x75:
        {
            uint8_t  eff = (op1 + x) & 0xFF;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        ADC $%02X,X @ %02X = %02X",            opcode, op1, op1, eff, val); break;
        }
        case 0x6D:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   ADC $%04X = %02X",                     opcode, op1, op2, abs_addr, val); break;
        }
        case 0x7D:
        {
            uint16_t eff = abs_addr + x;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   ADC $%04X,X @ %04X = %02X",            opcode, op1, op2, abs_addr, eff, val); break;
        }
        case 0x79:
        {
            uint16_t eff = abs_addr + y;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   ADC $%04X,Y @ %04X = %02X",            opcode, op1, op2, abs_addr, eff, val); break;
        }
        case 0x61:
        {
            uint8_t  zp  = (op1 + x) & 0xFF;
            uint16_t eff = (bus_read(bus, (zp + 1) & 0xFF) << 8) | bus_read(bus, zp);
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        ADC ($%02X,X) @ %02X = %04X = %02X",   opcode, op1, op1, zp, eff, val); break;
        }
        case 0x71:
        {
            uint16_t base = (bus_read(bus, (op1 + 1) & 0xFF) << 8) | bus_read(bus, op1);
            uint16_t eff  = base + y;
            uint8_t  val  = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        ADC ($%02X),Y = %04X @ %04X = %02X",   opcode, op1, op1, base, eff, val); break;
        }
        // SBC
        case 0xE9: snprintf(out, out_size, "%02X %02X        SBC #$%02X",                    opcode, op1, op1);        break;
        case 0xE5:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        SBC $%02X = %02X",                     opcode, op1, op1, val);   break;
        }
        case 0xF5:
        {
            uint8_t  eff = (op1 + x) & 0xFF;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        SBC $%02X,X @ %02X = %02X",            opcode, op1, op1, eff, val); break;
        }
        case 0xED:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   SBC $%04X = %02X",                     opcode, op1, op2, abs_addr, val); break;
        }
        case 0xFD:
        {
            uint16_t eff = abs_addr + x;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   SBC $%04X,X @ %04X = %02X",            opcode, op1, op2, abs_addr, eff, val); break;
        }
        case 0xF9:
        {
            uint16_t eff = abs_addr + y;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   SBC $%04X,Y @ %04X = %02X",            opcode, op1, op2, abs_addr, eff, val); break;
        }
        case 0xE1:
        {
            uint8_t  zp  = (op1 + x) & 0xFF;
            uint16_t eff = (bus_read(bus, (zp + 1) & 0xFF) << 8) | bus_read(bus, zp);
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        SBC ($%02X,X) @ %02X = %04X = %02X",   opcode, op1, op1, zp, eff, val); break;
        }
        case 0xF1:
        {
            uint16_t base = (bus_read(bus, (op1 + 1) & 0xFF) << 8) | bus_read(bus, op1);
            uint16_t eff  = base + y;
            uint8_t  val  = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        SBC ($%02X),Y = %04X @ %04X = %02X",   opcode, op1, op1, base, eff, val); break;
        }
        // AND
        case 0x29: snprintf(out, out_size, "%02X %02X        AND #$%02X",                    opcode, op1, op1);        break;
        case 0x25:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        AND $%02X = %02X",                     opcode, op1, op1, val);   break;
        }
        case 0x35:
        {
            uint8_t  eff = (op1 + x) & 0xFF;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        AND $%02X,X @ %02X = %02X",            opcode, op1, op1, eff, val); break;
        }
        case 0x2D:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   AND $%04X = %02X",                     opcode, op1, op2, abs_addr, val); break;
        }
        case 0x3D:
        {
            uint16_t eff = abs_addr + x;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   AND $%04X,X @ %04X = %02X",            opcode, op1, op2, abs_addr, eff, val); break;
        }
        case 0x39:
        {
            uint16_t eff = abs_addr + y;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   AND $%04X,Y @ %04X = %02X",            opcode, op1, op2, abs_addr, eff, val); break;
        }
        case 0x21:
        {
            uint8_t  zp  = (op1 + x) & 0xFF;
            uint16_t eff = (bus_read(bus, (zp + 1) & 0xFF) << 8) | bus_read(bus, zp);
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        AND ($%02X,X) @ %02X = %04X = %02X",   opcode, op1, op1, zp, eff, val); break;
        }
        case 0x31:
        {
            uint16_t base = (bus_read(bus, (op1 + 1) & 0xFF) << 8) | bus_read(bus, op1);
            uint16_t eff  = base + y;
            uint8_t  val  = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        AND ($%02X),Y = %04X @ %04X = %02X",   opcode, op1, op1, base, eff, val); break;
        }
        // EOR
        case 0x49: snprintf(out, out_size, "%02X %02X        EOR #$%02X",                    opcode, op1, op1);        break;
        case 0x45:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        EOR $%02X = %02X",                     opcode, op1, op1, val);   break;
        }
        case 0x55:
        {
            uint8_t  eff = (op1 + x) & 0xFF;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        EOR $%02X,X @ %02X = %02X",            opcode, op1, op1, eff, val); break;
        }
        case 0x4D:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   EOR $%04X = %02X",                     opcode, op1, op2, abs_addr, val); break;
        }
        case 0x5D:
        {
            uint16_t eff = abs_addr + x;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   EOR $%04X,X @ %04X = %02X",            opcode, op1, op2, abs_addr, eff, val); break;
        }
        case 0x59:
        {
            uint16_t eff = abs_addr + y;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   EOR $%04X,Y @ %04X = %02X",            opcode, op1, op2, abs_addr, eff, val); break;
        }
        case 0x41:
        {
            uint8_t  zp  = (op1 + x) & 0xFF;
            uint16_t eff = (bus_read(bus, (zp + 1) & 0xFF) << 8) | bus_read(bus, zp);
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        EOR ($%02X,X) @ %02X = %04X = %02X",   opcode, op1, op1, zp, eff, val); break;
        }
        case 0x51:
        {
            uint16_t base = (bus_read(bus, (op1 + 1) & 0xFF) << 8) | bus_read(bus, op1);
            uint16_t eff  = base + y;
            uint8_t  val  = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        EOR ($%02X),Y = %04X @ %04X = %02X",   opcode, op1, op1, base, eff, val); break;
        }
        // ORA
        case 0x09: snprintf(out, out_size, "%02X %02X        ORA #$%02X",                    opcode, op1, op1);        break;
        case 0x05:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        ORA $%02X = %02X",                     opcode, op1, op1, val);   break;
        }
        case 0x15:
        {
            uint8_t  eff = (op1 + x) & 0xFF;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        ORA $%02X,X @ %02X = %02X",            opcode, op1, op1, eff, val); break;
        }
        case 0x0D:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   ORA $%04X = %02X",                     opcode, op1, op2, abs_addr, val); break;
        }
        case 0x1D:
        {
            uint16_t eff = abs_addr + x;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   ORA $%04X,X @ %04X = %02X",            opcode, op1, op2, abs_addr, eff, val); break;
        }
        case 0x19:
        {
            uint16_t eff = abs_addr + y;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   ORA $%04X,Y @ %04X = %02X",            opcode, op1, op2, abs_addr, eff, val); break;
        }
        case 0x01:
        {
            uint8_t  zp  = (op1 + x) & 0xFF;
            uint16_t eff = (bus_read(bus, (zp + 1) & 0xFF) << 8) | bus_read(bus, zp);
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        ORA ($%02X,X) @ %02X = %04X = %02X",   opcode, op1, op1, zp, eff, val); break;
        }
        case 0x11:
        {
            uint16_t base = (bus_read(bus, (op1 + 1) & 0xFF) << 8) | bus_read(bus, op1);
            uint16_t eff  = base + y;
            uint8_t  val  = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        ORA ($%02X),Y = %04X @ %04X = %02X",   opcode, op1, op1, base, eff, val); break;
        }
        // ASL
        case 0x0A: snprintf(out, out_size, "%02X             ASL A",                          opcode);                       break;
        case 0x06:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        ASL $%02X = %02X",                     opcode, op1, op1, val);   break;
        }
        case 0x16:
        {
            uint8_t  eff = (op1 + x) & 0xFF;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        ASL $%02X,X @ %02X = %02X",            opcode, op1, op1, eff, val); break;
        }
        case 0x0E:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   ASL $%04X = %02X",                     opcode, op1, op2, abs_addr, val); break;
        }
        case 0x1E:
        {
            uint16_t eff = abs_addr + x;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   ASL $%04X,X @ %04X = %02X",            opcode, op1, op2, abs_addr, eff, val); break;
        }
        // LSR
        case 0x4A: snprintf(out, out_size, "%02X             LSR A",                          opcode);                       break;
        case 0x46:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        LSR $%02X = %02X",                     opcode, op1, op1, val);   break;
        }
        case 0x56:
        {
            uint8_t  eff = (op1 + x) & 0xFF;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        LSR $%02X,X @ %02X = %02X",            opcode, op1, op1, eff, val); break;
        }
        case 0x4E:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   LSR $%04X = %02X",                     opcode, op1, op2, abs_addr, val); break;
        }
        case 0x5E:
        {
            uint16_t eff = abs_addr + x;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   LSR $%04X,X @ %04X = %02X",            opcode, op1, op2, abs_addr, eff, val); break;
        }
        // ROL
        case 0x2A: snprintf(out, out_size, "%02X             ROL A",                          opcode);                       break;
        case 0x26:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        ROL $%02X = %02X",                     opcode, op1, op1, val);   break;
        }
        case 0x36:
        {
            uint8_t  eff = (op1 + x) & 0xFF;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        ROL $%02X,X @ %02X = %02X",            opcode, op1, op1, eff, val); break;
        }
        case 0x2E:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   ROL $%04X = %02X",                     opcode, op1, op2, abs_addr, val); break;
        }
        case 0x3E:
        {
            uint16_t eff = abs_addr + x;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   ROL $%04X,X @ %04X = %02X",            opcode, op1, op2, abs_addr, eff, val); break;
        }
        // ROR
        case 0x6A: snprintf(out, out_size, "%02X             ROR A",                          opcode);                       break;
        case 0x66:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        ROR $%02X = %02X",                     opcode, op1, op1, val);   break;
        }
        case 0x76:
        {
            uint8_t  eff = (op1 + x) & 0xFF;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        ROR $%02X,X @ %02X = %02X",            opcode, op1, op1, eff, val); break;
        }
        case 0x6E:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   ROR $%04X = %02X",                     opcode, op1, op2, abs_addr, val); break;
        }
        case 0x7E:
        {
            uint16_t eff = abs_addr + x;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   ROR $%04X,X @ %04X = %02X",            opcode, op1, op2, abs_addr, eff, val); break;
        }
        // SETS + CLEARS
        case 0x18: snprintf(out, out_size, "%02X             CLC",               opcode);                       break;
        case 0xD8: snprintf(out, out_size, "%02X             CLD",               opcode);                       break;
        case 0x58: snprintf(out, out_size, "%02X             CLI",               opcode);                       break;
        case 0xB8: snprintf(out, out_size, "%02X             CLV",               opcode);                       break;
        case 0x38: snprintf(out, out_size, "%02X             SEC",               opcode);                       break;
        case 0xF8: snprintf(out, out_size, "%02X             SED",               opcode);                       break;
        case 0x78: snprintf(out, out_size, "%02X             SEI",               opcode);                       break;
        // CMP
        case 0xC9: snprintf(out, out_size, "%02X %02X        CMP #$%02X",                    opcode, op1, op1);        break;
        case 0xC5:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        CMP $%02X = %02X",                     opcode, op1, op1, val);   break;
        }
        case 0xD5:
        {
            uint8_t  eff = (op1 + x) & 0xFF;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        CMP $%02X,X @ %02X = %02X",            opcode, op1, op1, eff, val); break;
        }
        case 0xCD:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   CMP $%04X = %02X",                     opcode, op1, op2, abs_addr, val); break;
        }
        case 0xDD:
        {
            uint16_t eff = abs_addr + x;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   CMP $%04X,X @ %04X = %02X",            opcode, op1, op2, abs_addr, eff, val); break;
        }
        case 0xD9:
        {
            uint16_t eff = abs_addr + y;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   CMP $%04X,Y @ %04X = %02X",            opcode, op1, op2, abs_addr, eff, val); break;
        }
        case 0xC1:
        {
            uint8_t  zp  = (op1 + x) & 0xFF;
            uint16_t eff = (bus_read(bus, (zp + 1) & 0xFF) << 8) | bus_read(bus, zp);
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        CMP ($%02X,X) @ %02X = %04X = %02X",   opcode, op1, op1, zp, eff, val); break;
        }
        case 0xD1:
        {
            uint16_t base = (bus_read(bus, (op1 + 1) & 0xFF) << 8) | bus_read(bus, op1);
            uint16_t eff  = base + y;
            uint8_t  val  = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        CMP ($%02X),Y = %04X @ %04X = %02X",   opcode, op1, op1, base, eff, val); break;
        }
        // CPX
        case 0xE0: snprintf(out, out_size, "%02X %02X        CPX #$%02X",                    opcode, op1, op1);        break;
        case 0xE4:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        CPX $%02X = %02X",                     opcode, op1, op1, val);   break;
        }
        case 0xEC:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   CPX $%04X = %02X",                     opcode, op1, op2, abs_addr, val); break;
        }
        // CPY
        case 0xC0: snprintf(out, out_size, "%02X %02X        CPY #$%02X",                    opcode, op1, op1);        break;
        case 0xC4:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        CPY $%02X = %02X",                     opcode, op1, op1, val);   break;
        }
        case 0xCC:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   CPY $%04X = %02X",                     opcode, op1, op2, abs_addr, val); break;
        }
        // BIT
        case 0x24:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        BIT $%02X = %02X",                     opcode, op1, op1, val);   break;
        }
        case 0x2C:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   BIT $%04X = %02X",                     opcode, op1, op2, abs_addr, val); break;
        }
        // BRANCHES
        case 0x90: snprintf(out, out_size, "%02X %02X        BCC $%04X", opcode, op1, (uint16_t)(pc + 2 + (int8_t)op1)); break;
        case 0xB0: snprintf(out, out_size, "%02X %02X        BCS $%04X", opcode, op1, (uint16_t)(pc + 2 + (int8_t)op1)); break;
        case 0xF0: snprintf(out, out_size, "%02X %02X        BEQ $%04X", opcode, op1, (uint16_t)(pc + 2 + (int8_t)op1)); break;
        case 0x30: snprintf(out, out_size, "%02X %02X        BMI $%04X", opcode, op1, (uint16_t)(pc + 2 + (int8_t)op1)); break;
        case 0xD0: snprintf(out, out_size, "%02X %02X        BNE $%04X", opcode, op1, (uint16_t)(pc + 2 + (int8_t)op1)); break;
        case 0x10: snprintf(out, out_size, "%02X %02X        BPL $%04X", opcode, op1, (uint16_t)(pc + 2 + (int8_t)op1)); break;
        case 0x50: snprintf(out, out_size, "%02X %02X        BVC $%04X", opcode, op1, (uint16_t)(pc + 2 + (int8_t)op1)); break;
        case 0x70: snprintf(out, out_size, "%02X %02X        BVS $%04X", opcode, op1, (uint16_t)(pc + 2 + (int8_t)op1)); break;
        // JUMPS
        case 0x4C: snprintf(out, out_size, "%02X %02X %02X   JMP $%04X",         opcode, op1, op2,  abs_addr);  break;
        case 0x6C:
        {
            uint16_t ptr = abs_addr;
            uint16_t eff = (bus_read(bus, (ptr & 0xFF00) | ((ptr + 1) & 0x00FF)) << 8) | bus_read(bus, ptr);
            snprintf(out, out_size, "%02X %02X %02X   JMP ($%04X) = %04X",       opcode, op1, op2, abs_addr, eff); break;
        }
        case 0x20: snprintf(out, out_size, "%02X %02X %02X   JSR $%04X",         opcode, op1, op2,  abs_addr);  break;
        case 0x60: snprintf(out, out_size, "%02X             RTS",               opcode);                       break;
        // INTERRUPTS
        case 0x00: snprintf(out, out_size, "%02X             BRK",               opcode);                       break;
        case 0x40: snprintf(out, out_size, "%02X             RTI",               opcode);                       break;
        // NOP
        case 0xEA: snprintf(out, out_size, "%02X             NOP",               opcode);                       break;



        // ---- ILLEGAL OPCODES ----
 
        // NOP variants (implied)
        case 0x1A:
        case 0x3A:
        case 0x5A:
        case 0x7A:
        case 0xDA:
        case 0xFA:
            snprintf(out, out_size, "%02X             NOP*",              opcode);                       break;
 
        // NOP variants (immediate)
        case 0x80:
        case 0x82:
        case 0x89:
        case 0xC2:
        case 0xE2:
            snprintf(out, out_size, "%02X %02X        NOP* #$%02X",                   opcode, op1, op1);        break;
 
        // NOP variants (zero page)
        case 0x04:
        case 0x44:
        case 0x64:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        NOP* $%02X = %02X",             opcode, op1, op1, val);   break;
        }
 
        // NOP variants (zero page, X)
        case 0x14:
        case 0x34:
        case 0x54:
        case 0x74:
        case 0xD4:
        case 0xF4:
        {
            uint8_t  eff = (op1 + x) & 0xFF;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        NOP* $%02X,X @ %02X = %02X",   opcode, op1, op1, eff, val); break;
        }
 
        // NOP variants (absolute)
        case 0x0C:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   NOP* $%04X = %02X",            opcode, op1, op2, abs_addr, val); break;
        }
 
        // NOP variants (absolute, X)
        case 0x1C:
        case 0x3C:
        case 0x5C:
        case 0x7C:
        case 0xDC:
        case 0xFC:
        {
            uint16_t eff = abs_addr + x;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   NOP* $%04X,X @ %04X = %02X",  opcode, op1, op2, abs_addr, eff, val); break;
        }
 
        // ALR #imm  (AND #imm then LSR A)
        case 0x4B:
            snprintf(out, out_size, "%02X %02X        ALR* #$%02X",                  opcode, op1, op1);        break;
 
        // ANC #imm
        case 0x0B:
            snprintf(out, out_size, "%02X %02X        ANC* #$%02X",                  opcode, op1, op1);        break;
 
        // ANC2 #imm
        case 0x2B:
            snprintf(out, out_size, "%02X %02X        ANC2* #$%02X",                 opcode, op1, op1);        break;
 
        // DCP  (DEC then CMP)
        case 0xC7:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        DCP* $%02X = %02X",            opcode, op1, op1, val);   break;
        }
        case 0xD7:
        {
            uint8_t  eff = (op1 + x) & 0xFF;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        DCP* $%02X,X @ %02X = %02X",   opcode, op1, op1, eff, val); break;
        }
        case 0xCF:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   DCP* $%04X = %02X",            opcode, op1, op2, abs_addr, val); break;
        }
        case 0xDF:
        {
            uint16_t eff = abs_addr + x;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   DCP* $%04X,X @ %04X = %02X",  opcode, op1, op2, abs_addr, eff, val); break;
        }
        case 0xDB:
        {
            uint16_t eff = abs_addr + y;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   DCP* $%04X,Y @ %04X = %02X",  opcode, op1, op2, abs_addr, eff, val); break;
        }
        case 0xC3:
        {
            uint8_t  zp  = (op1 + x) & 0xFF;
            uint16_t eff = (bus_read(bus, (zp + 1) & 0xFF) << 8) | bus_read(bus, zp);
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        DCP* ($%02X,X) @ %02X = %04X = %02X", opcode, op1, op1, zp, eff, val); break;
        }
        case 0xD3:
        {
            uint16_t base = (bus_read(bus, (op1 + 1) & 0xFF) << 8) | bus_read(bus, op1);
            uint16_t eff  = base + y;
            uint8_t  val  = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        DCP* ($%02X),Y = %04X @ %04X = %02X", opcode, op1, op1, base, eff, val); break;
        }
 
        // ISC  (INC then SBC)
        case 0xE7:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        ISC* $%02X = %02X",            opcode, op1, op1, val);   break;
        }
        case 0xF7:
        {
            uint8_t  eff = (op1 + x) & 0xFF;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        ISC* $%02X,X @ %02X = %02X",   opcode, op1, op1, eff, val); break;
        }
        case 0xEF:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   ISC* $%04X = %02X",            opcode, op1, op2, abs_addr, val); break;
        }
        case 0xFF:
        {
            uint16_t eff = abs_addr + x;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   ISC* $%04X,X @ %04X = %02X",  opcode, op1, op2, abs_addr, eff, val); break;
        }
        case 0xFB:
        {
            uint16_t eff = abs_addr + y;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   ISC* $%04X,Y @ %04X = %02X",  opcode, op1, op2, abs_addr, eff, val); break;
        }
        case 0xE3:
        {
            uint8_t  zp  = (op1 + x) & 0xFF;
            uint16_t eff = (bus_read(bus, (zp + 1) & 0xFF) << 8) | bus_read(bus, zp);
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        ISC* ($%02X,X) @ %02X = %04X = %02X", opcode, op1, op1, zp, eff, val); break;
        }
        case 0xF3:
        {
            uint16_t base = (bus_read(bus, (op1 + 1) & 0xFF) << 8) | bus_read(bus, op1);
            uint16_t eff  = base + y;
            uint8_t  val  = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        ISC* ($%02X),Y = %04X @ %04X = %02X", opcode, op1, op1, base, eff, val); break;
        }
 
        // LAX  (LDA + LDX)
        case 0xA7:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        LAX* $%02X = %02X",            opcode, op1, op1, val);   break;
        }
        case 0xB7:
        {
            uint8_t  eff = (op1 + y) & 0xFF;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        LAX* $%02X,Y @ %02X = %02X",   opcode, op1, op1, eff, val); break;
        }
        case 0xAF:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   LAX* $%04X = %02X",            opcode, op1, op2, abs_addr, val); break;
        }
        case 0xBF:
        {
            uint16_t eff = abs_addr + y;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   LAX* $%04X,Y @ %04X = %02X",  opcode, op1, op2, abs_addr, eff, val); break;
        }
        case 0xA3:
        {
            uint8_t  zp  = (op1 + x) & 0xFF;
            uint16_t eff = (bus_read(bus, (zp + 1) & 0xFF) << 8) | bus_read(bus, zp);
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        LAX* ($%02X,X) @ %02X = %04X = %02X", opcode, op1, op1, zp, eff, val); break;
        }
        case 0xB3:
        {
            uint16_t base = (bus_read(bus, (op1 + 1) & 0xFF) << 8) | bus_read(bus, op1);
            uint16_t eff  = base + y;
            uint8_t  val  = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        LAX* ($%02X),Y = %04X @ %04X = %02X", opcode, op1, op1, base, eff, val); break;
        }
 
        // RLA  (ROL then AND)
        case 0x27:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        RLA* $%02X = %02X",            opcode, op1, op1, val);   break;
        }
        case 0x37:
        {
            uint8_t  eff = (op1 + x) & 0xFF;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        RLA* $%02X,X @ %02X = %02X",   opcode, op1, op1, eff, val); break;
        }
        case 0x2F:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   RLA* $%04X = %02X",            opcode, op1, op2, abs_addr, val); break;
        }
        case 0x3F:
        {
            uint16_t eff = abs_addr + x;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   RLA* $%04X,X @ %04X = %02X",  opcode, op1, op2, abs_addr, eff, val); break;
        }
        case 0x3B:
        {
            uint16_t eff = abs_addr + y;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   RLA* $%04X,Y @ %04X = %02X",  opcode, op1, op2, abs_addr, eff, val); break;
        }
        case 0x23:
        {
            uint8_t  zp  = (op1 + x) & 0xFF;
            uint16_t eff = (bus_read(bus, (zp + 1) & 0xFF) << 8) | bus_read(bus, zp);
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        RLA* ($%02X,X) @ %02X = %04X = %02X", opcode, op1, op1, zp, eff, val); break;
        }
        case 0x33:
        {
            uint16_t base = (bus_read(bus, (op1 + 1) & 0xFF) << 8) | bus_read(bus, op1);
            uint16_t eff  = base + y;
            uint8_t  val  = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        RLA* ($%02X),Y = %04X @ %04X = %02X", opcode, op1, op1, base, eff, val); break;
        }
 
        // RRA  (ROR then ADC)
        case 0x67:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        RRA* $%02X = %02X",            opcode, op1, op1, val);   break;
        }
        case 0x77:
        {
            uint8_t  eff = (op1 + x) & 0xFF;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        RRA* $%02X,X @ %02X = %02X",   opcode, op1, op1, eff, val); break;
        }
        case 0x6F:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   RRA* $%04X = %02X",            opcode, op1, op2, abs_addr, val); break;
        }
        case 0x7F:
        {
            uint16_t eff = abs_addr + x;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   RRA* $%04X,X @ %04X = %02X",  opcode, op1, op2, abs_addr, eff, val); break;
        }
        case 0x7B:
        {
            uint16_t eff = abs_addr + y;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   RRA* $%04X,Y @ %04X = %02X",  opcode, op1, op2, abs_addr, eff, val); break;
        }
        case 0x63:
        {
            uint8_t  zp  = (op1 + x) & 0xFF;
            uint16_t eff = (bus_read(bus, (zp + 1) & 0xFF) << 8) | bus_read(bus, zp);
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        RRA* ($%02X,X) @ %02X = %04X = %02X", opcode, op1, op1, zp, eff, val); break;
        }
        case 0x73:
        {
            uint16_t base = (bus_read(bus, (op1 + 1) & 0xFF) << 8) | bus_read(bus, op1);
            uint16_t eff  = base + y;
            uint8_t  val  = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        RRA* ($%02X),Y = %04X @ %04X = %02X", opcode, op1, op1, base, eff, val); break;
        }
 
        // SAX  (store A & X)
        case 0x87:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        SAX* $%02X = %02X",            opcode, op1, op1, val);   break;
        }
        case 0x97:
        {
            uint8_t  eff = (op1 + y) & 0xFF;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        SAX* $%02X,Y @ %02X = %02X",   opcode, op1, op1, eff, val); break;
        }
        case 0x8F:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   SAX* $%04X = %02X",            opcode, op1, op2, abs_addr, val); break;
        }
        case 0x83:
        {
            uint8_t  zp  = (op1 + x) & 0xFF;
            uint16_t eff = (bus_read(bus, (zp + 1) & 0xFF) << 8) | bus_read(bus, zp);
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        SAX* ($%02X,X) @ %02X = %04X = %02X", opcode, op1, op1, zp, eff, val); break;
        }
 
        // SLO  (ASL then ORA)
        case 0x07:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        SLO* $%02X = %02X",            opcode, op1, op1, val);   break;
        }
        case 0x17:
        {
            uint8_t  eff = (op1 + x) & 0xFF;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        SLO* $%02X,X @ %02X = %02X",   opcode, op1, op1, eff, val); break;
        }
        case 0x0F:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   SLO* $%04X = %02X",            opcode, op1, op2, abs_addr, val); break;
        }
        case 0x1F:
        {
            uint16_t eff = abs_addr + x;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   SLO* $%04X,X @ %04X = %02X",  opcode, op1, op2, abs_addr, eff, val); break;
        }
        case 0x1B:
        {
            uint16_t eff = abs_addr + y;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   SLO* $%04X,Y @ %04X = %02X",  opcode, op1, op2, abs_addr, eff, val); break;
        }
        case 0x03:
        {
            uint8_t  zp  = (op1 + x) & 0xFF;
            uint16_t eff = (bus_read(bus, (zp + 1) & 0xFF) << 8) | bus_read(bus, zp);
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        SLO* ($%02X,X) @ %02X = %04X = %02X", opcode, op1, op1, zp, eff, val); break;
        }
        case 0x13:
        {
            uint16_t base = (bus_read(bus, (op1 + 1) & 0xFF) << 8) | bus_read(bus, op1);
            uint16_t eff  = base + y;
            uint8_t  val  = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        SLO* ($%02X),Y = %04X @ %04X = %02X", opcode, op1, op1, base, eff, val); break;
        }
 
        // SRE  (LSR then EOR)
        case 0x47:
        {
            uint8_t val = bus_read(bus, op1);
            snprintf(out, out_size, "%02X %02X        SRE* $%02X = %02X",            opcode, op1, op1, val);   break;
        }
        case 0x57:
        {
            uint8_t  eff = (op1 + x) & 0xFF;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        SRE* $%02X,X @ %02X = %02X",   opcode, op1, op1, eff, val); break;
        }
        case 0x4F:
        {
            uint8_t val = bus_read(bus, abs_addr);
            snprintf(out, out_size, "%02X %02X %02X   SRE* $%04X = %02X",            opcode, op1, op2, abs_addr, val); break;
        }
        case 0x5F:
        {
            uint16_t eff = abs_addr + x;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   SRE* $%04X,X @ %04X = %02X",  opcode, op1, op2, abs_addr, eff, val); break;
        }
        case 0x5B:
        {
            uint16_t eff = abs_addr + y;
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X %02X   SRE* $%04X,Y @ %04X = %02X",  opcode, op1, op2, abs_addr, eff, val); break;
        }
        case 0x43:
        {
            uint8_t  zp  = (op1 + x) & 0xFF;
            uint16_t eff = (bus_read(bus, (zp + 1) & 0xFF) << 8) | bus_read(bus, zp);
            uint8_t  val = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        SRE* ($%02X,X) @ %02X = %04X = %02X", opcode, op1, op1, zp, eff, val); break;
        }
        case 0x53:
        {
            uint16_t base = (bus_read(bus, (op1 + 1) & 0xFF) << 8) | bus_read(bus, op1);
            uint16_t eff  = base + y;
            uint8_t  val  = bus_read(bus, eff);
            snprintf(out, out_size, "%02X %02X        SRE* ($%02X),Y = %04X @ %04X = %02X", opcode, op1, op1, base, eff, val); break;
        }
 
        default:   snprintf(out, out_size, "%02X             ???",               opcode);                       break;
    }
}