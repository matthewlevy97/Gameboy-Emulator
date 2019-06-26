#include "cpu.h"
#include "memory.h"
#include "lcd.h"
#include "graphics.h"
#include "rom.h"

/**
Global variables
*/
struct cpu_state cpu_state;

/**
Static Functions
*/
static void cpu_rom_reset();

static unsigned char parse_opcode(unsigned char opcode);
static unsigned char parse_prefixed_opcode(unsigned char opcode);

static void do_cp(unsigned char val);

/**
Static Variables
*/
static struct registers * cpu_registers;
static char * interrupt_register;

#ifdef DISASSEMBLE
char disassembly[256];
short disassembly_pc;
#endif

/**
Functions
*/
void cpu_init() {	
	cpu_registers = &cpu_state.registers;
	
	interrupt_register = memory_dump() + 0xFFFF;

	cpu_reset();
}

void cpu_reset() {
	// Reset everything
	memset(&cpu_state, 0, sizeof(struct cpu_state));
	cpu_state.running = 1;

#ifdef IGNORE_BOOTLOADER
	cpu_registers->PC = 0x100;
	cpu_rom_reset();
#endif

}

struct cpu_state cpu_getState() {
	return cpu_state;
}
void cpu_setState(struct cpu_state state) {
	cpu_state = state;
}

unsigned char cpu_step() {
	unsigned char cycles, byte;

#ifndef IGNORE_BOOTLOADER
	if(cpu_registers->PC == 0x100)
		rom_set_preamble();
#endif
	
	byte = memory_read8(cpu_registers->PC++);
#ifdef DISASSEMBLE
	disassembly_pc = cpu_registers->PC - 1;
	memset(disassembly, 0, sizeof(disassembly));
#endif
	
	cycles = parse_opcode(byte);
	cpu_state.total_cycles += cycles;
	
	// Update peripherals
	lcd_update(cycles);
	graphics_update();
	
	// If no cycles, there is a problem
	if(!cycles)
		cpu_state.running = 0;
	
	return cycles;
}

static unsigned char parse_opcode(unsigned char opcode) {
	unsigned char cycles;
	unsigned char tmp_c;
	unsigned short tmp_s;
	
	switch(opcode)
	{
		case 0x00:
			// NOP
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "NOP");
#endif
			break;
		case 0x01:
			// LD BC, nn
			cpu_registers->BC = memory_read16(cpu_registers->PC);
			cpu_registers->PC += 2;
			cycles = 12;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD BC, $%04x", cpu_registers->BC);
#endif
			break;
		case 0x02:
			// LD (BC), A
			memory_write8(cpu_registers->BC, cpu_registers->A);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD (BC), A");
#endif
			break;
		case 0x03:
			// INC BC
			cpu_registers->BC++;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "INC BC");
#endif
			break;
		case 0x04:
			// INC B
			cpu_registers->B++;
			cpu_registers->FLAG = FLAG_COMPUTE_INC(cpu_registers->B) | GET_BIT(cpu_registers->FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "INC B");
#endif
			break;
		case 0x05:
			// DEC B
			cpu_registers->B--;
			cpu_registers->FLAG = FLAG_COMPUTE_DEC(cpu_registers->B) | GET_BIT(cpu_registers->FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "DEC B");
#endif
			break;
		case 0x06:
			// LD B, n
			cpu_registers->B = memory_read8(cpu_registers->PC++);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD B, $%02x", cpu_registers->B);
#endif
			break;
		case 0x07:
			// RLCA
			// TODO: Confirm this works
			tmp_c = (cpu_registers->A >> 7) & 1;
			cpu_registers->A <<= 0x1;
			cpu_registers->A |= (cpu_registers->FLAG >> C_FLAG) & 0x1;
			cpu_registers->FLAG = (cpu_registers->A ? 0 : 1<<Z_FLAG);
			cpu_registers->FLAG |= (tmp_c << C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RLCA");
#endif
			break;
		case 0x08:
			// LD (nn), SP
			tmp_s = memory_read16(cpu_registers->PC);
			cpu_registers->PC += 2;
			memory_write16(tmp_s, cpu_registers->SP);
			cycles = 20;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD ($%04x), SP", tmp_s);
#endif
			break;
		case 0x0A:
			// LD A, (BC)
			cpu_registers->A = memory_read8(cpu_registers->BC);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD A, (BC)");
#endif
			break;
		case 0x0B:
			// DEC BC
			cpu_registers->BC--;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "DEC BC");
#endif
			break;
		case 0x0C:
			// INC C
			cpu_registers->C++;
			cpu_registers->FLAG = FLAG_COMPUTE_INC(cpu_registers->C) | GET_BIT(cpu_registers->FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "INC C");
#endif
			break;
		case 0x0D:
			// DEC C
			cpu_registers->C--;
			cpu_registers->FLAG = FLAG_COMPUTE_DEC(cpu_registers->C) | GET_BIT(cpu_registers->FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "DEC C");
#endif
			break;
		case 0x0E:
			// LD C, n
			cpu_registers->C = memory_read8(cpu_registers->PC++);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD C, $%02x", cpu_registers->C);
#endif
			break;
		case 0x0F:
			// RRCA
			tmp_c = cpu_registers->A & 0x1; // Far right
			cpu_registers->A >>= 1; // Drop far right
			cpu_registers->A |= (tmp_c << 7); // Set far left
			cpu_registers->FLAG = (cpu_registers->A ? 0 : 1<<Z_FLAG);
			cpu_registers->FLAG |= (tmp_c << C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RRCA");
#endif
			break;

		
		case 0x11:
			// LD DE, nn
			cpu_registers->DE = memory_read16(cpu_registers->PC);
			cpu_registers->PC += 2;
			cycles = 12;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD DE, $%04x", cpu_registers->DE);
#endif
			break;
		case 0x12:
			// LD (DE), A
			memory_write8(cpu_registers->DE, cpu_registers->A);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD (DE), A");
#endif
			break;
		case 0x13:
			// INC DE
			cpu_registers->DE++;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "INC DE");
#endif
			break;
		case 0x14:
			// INC D
			cpu_registers->D++;
			cpu_registers->FLAG = FLAG_COMPUTE_INC(cpu_registers->D) | GET_BIT(cpu_registers->FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "INC D");
#endif
			break;
		case 0x15:
			// DEC D
			cpu_registers->D--;
			cpu_registers->FLAG = FLAG_COMPUTE_DEC(cpu_registers->D) | GET_BIT(cpu_registers->FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "DEC D");
#endif
			break;
		case 0x16:
			// LD D, n
			cpu_registers->D = memory_read8(cpu_registers->PC++);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD D, $%02x", cpu_registers->D);
#endif
			break;
		case 0x17:
			// RLA
			// See 0xCB 0x17 for more info on this algorithm
			tmp_c = (cpu_registers->A & 128) >> (Z_FLAG - C_FLAG);
			cpu_registers->A = (cpu_registers->A << 1) | GET_BIT(cpu_registers->FLAG, C_FLAG);
			cpu_registers->FLAG = tmp_c;
			
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RLA");
#endif
			break;
		case 0x18:
			// JR n
			tmp_c = memory_read8(cpu_registers->PC);
			cpu_registers->PC++;
			
			cpu_registers->PC += (signed char)tmp_c;
			
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "JR %d", tmp_c);
#endif
			break;
		case 0x1A:
			// LD A, (DE)
			cpu_registers->A = memory_read8(cpu_registers->DE);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD A, (DE)");
#endif
			break;
		case 0x1B:
			// DEC DE
			cpu_registers->DE--;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "DEC DE");
#endif
			break;
		case 0x1C:
			// INC E
			cpu_registers->E++;
			cpu_registers->FLAG = FLAG_COMPUTE_INC(cpu_registers->E) | GET_BIT(cpu_registers->FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "INC E");
#endif
			break;
		case 0x1D:
			// DEC E
			cpu_registers->E--;
			cpu_registers->FLAG = FLAG_COMPUTE_DEC(cpu_registers->E) | GET_BIT(cpu_registers->FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "DEC E");
#endif
			break;
		case 0x1E:
			// LD E, n
			cpu_registers->E = memory_read8(cpu_registers->PC++);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD E, $%02x", cpu_registers->E);
#endif
			break;
		case 0x1F:
			// RRA
			tmp_c = cpu_registers->A & 0x1; // Far right
			tmp_s = (cpu_registers->FLAG >> C_FLAG) & 0x1; // C
			cpu_registers->A >>= 1; // Drop far right
			cpu_registers->A |= (tmp_s << 7); // Set far left
			cpu_registers->FLAG = (cpu_registers->A ? 0 : 1<<Z_FLAG);
			cpu_registers->FLAG |= (tmp_s << C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RRA");
#endif
			break;
		
		case 0x20:
			// JR NZ, *
			tmp_c = memory_read16(cpu_registers->PC++);
			
			// Jump if Z-flag is reset
			if(!GET_BIT(cpu_registers->FLAG, Z_FLAG))
				cpu_registers->PC += (signed char)tmp_c;
			
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "JR NZ, %02d", tmp_c);
#endif
			break;
		case 0x21:
			// LD HL, nn
			cpu_registers->HL = memory_read16(cpu_registers->PC);
			cpu_registers->PC += 2;
			cycles = 12;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD HL, $%04x", cpu_registers->HL);
#endif
			break;
		case 0x22:
			// LD (HL+), A
			memory_write8(cpu_registers->HL++, cpu_registers->A);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD (HL+), A");
#endif
			break;
		case 0x23:
			// INC HL
			cpu_registers->HL++;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "INC HL");
#endif
			break;
		case 0x24:
			// INC H
			cpu_registers->H++;
			cpu_registers->FLAG = FLAG_COMPUTE_INC(cpu_registers->H) | GET_BIT(cpu_registers->FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "INC H");
#endif
			break;
		case 0x25:
			// DEC H
			cpu_registers->H--;
			cpu_registers->FLAG = FLAG_COMPUTE_DEC(cpu_registers->H) | GET_BIT(cpu_registers->FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "DEC H");
#endif
			break;
		case 0x26:
			// LD H, n
			cpu_registers->H = memory_read8(cpu_registers->PC++);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD H, $%02x", cpu_registers->H);
#endif
			break;
		case 0x28:
			// JR Z, *
			tmp_c = memory_read8(cpu_registers->PC++);
			
			// Jump if Z-flag is set
			if(GET_BIT(cpu_registers->FLAG, Z_FLAG))
				cpu_registers->PC += (signed char)tmp_c;
			
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "JR Z, %02d", tmp_c);
#endif
			break;
		case 0x2A:
			// LD A, (HL+)
			cpu_registers->A = memory_read8(cpu_registers->HL++);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD A, (HL+)");
#endif
			break;
		case 0x2B:
			// DEC HL
			cpu_registers->HL--;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "DEC HL");
#endif
			break;
		case 0x2C:
			// INC L
			cpu_registers->L++;
			cpu_registers->FLAG = FLAG_COMPUTE_INC(cpu_registers->L) | GET_BIT(cpu_registers->FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "INC L");
#endif
			break;
		case 0x2D:
			// DEC L
			cpu_registers->L--;
			cpu_registers->FLAG = FLAG_COMPUTE_DEC(cpu_registers->L) | GET_BIT(cpu_registers->FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "DEC L");
#endif
			break;
		case 0x2E:
			// LD L, n
			cpu_registers->L = memory_read8(cpu_registers->PC++);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD L, $%02x", cpu_registers->L);
#endif
			break;
		case 0x2F:
			// CPL
			cpu_registers->A = ~cpu_registers->A;
			cpu_registers->FLAG |= FLAG_PRECOMPUTE_CPL;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "CPL");
#endif
			break;
		
		case 0x30:
			// JR NC, *
			tmp_c = memory_read8(cpu_registers->PC++);
			
			// Jump if C-flag is reset
			if(!GET_BIT(cpu_registers->FLAG, C_FLAG))
				cpu_registers->PC += (signed char)tmp_c;
			
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "JR NC, %02d", tmp_c);
#endif
			break;
		case 0x31:
			// LD SP,$aabb
			cpu_registers->SP = memory_read16(cpu_registers->PC);
			cpu_registers->PC += 2;
			cycles = 12;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD SP, $%04x", cpu_registers->SP);
#endif
			break;
		case 0x32:
			// LD (HL-), A
			memory_write8(cpu_registers->HL--, cpu_registers->A);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD (HL-), A");
#endif
			break;
		case 0x33:
			// INC SP
			cpu_registers->SP++;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "INC SP");
#endif
			break;
		case 0x34:
			// INC (HL)
			tmp_c = memory_read8(cpu_registers->HL) + 1;
			memory_write8(cpu_registers->HL, tmp_c);
			cpu_registers->FLAG = FLAG_COMPUTE_INC(tmp_c) | GET_BIT(cpu_registers->FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "INC (HL)");
#endif
			break;
		case 0x35:
			// DEC (HL)
			tmp_c = memory_read8(cpu_registers->HL);
			memory_write8(cpu_registers->HL, --tmp_c);
			cpu_registers->FLAG = FLAG_COMPUTE_DEC(tmp_c) | GET_BIT(cpu_registers->FLAG, C_FLAG);
			cycles = 12;
#ifdef DISASSEMBLE
			sprintf(disassembly, "DEC (HL)");
#endif
			break;
		case 0x36:
			// LD (HL), n
			tmp_c = memory_read8(cpu_registers->PC++);
			memory_write8(cpu_registers->HL, tmp_c);
			cycles = 12;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD (HL), $%04x", tmp_c);
#endif
			break;
		case 0x37:
			// SCF
			cpu_registers->FLAG &= (1 << Z_FLAG);
			cpu_registers->FLAG |= (1 << C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "SCF");
#endif
			break;
		case 0x38:
			// JR C, *
			tmp_c = memory_read8(cpu_registers->PC++);
			
			// Jump if C-flag is set
			if(GET_BIT(cpu_registers->FLAG, C_FLAG))
				cpu_registers->PC += (signed char)tmp_c;
			
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "JR C, %02d", tmp_c);
#endif
			break;
		case 0x3A:
			// LD A, (HL-)
			tmp_s = memory_read8(cpu_registers->HL--);
			cpu_registers->A = tmp_s;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD A, (HL-)");
#endif
			break;
		case 0x3B:
			// DEC SP
			cpu_registers->SP--;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "DEC SP");
#endif
			break;
		case 0x3C:
			// INC A
			cpu_registers->A++;
			cpu_registers->FLAG = FLAG_COMPUTE_INC(cpu_registers->A) | GET_BIT(cpu_registers->FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "INC A");
#endif
			break;
		case 0x3D:
			// DEC A
			cpu_registers->A--;
			cpu_registers->FLAG = FLAG_COMPUTE_DEC(cpu_registers->A) | GET_BIT(cpu_registers->FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "DEC A");
#endif
			break;
		case 0x3E:
			// LD A, #
			cpu_registers->A = memory_read8(cpu_registers->PC++);
			
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD A, $%02x", cpu_registers->A);
#endif
			break;
		case 0x3F:
			// CCF
			cpu_registers->FLAG &= (1 << Z_FLAG);
			cpu_registers->FLAG ^= (1 << C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "CCF");
#endif
			break;
		
		case 0x40:
			// LD B, B
			// Yep. This is a thing
			cpu_registers->B = cpu_registers->B;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD B, B");
#endif
			break;
		case 0x41:
			// LD B, C
			cpu_registers->B = cpu_registers->C;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD B, C");
#endif
			break;
		case 0x42:
			// LD B, D
			cpu_registers->B = cpu_registers->D;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD B, D");
#endif
			break;
		case 0x43:
			// LD B, E
			cpu_registers->B = cpu_registers->E;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD B, E");
#endif
			break;
		case 0x44:
			// LD B, H
			cpu_registers->B = cpu_registers->H;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD B, H");
#endif
			break;
		case 0x45:
			// LD B, L
			cpu_registers->B = cpu_registers->L;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD B, L");
#endif
			break;
		case 0x46:
			// LD B, (HL)
			tmp_c = memory_read8(cpu_registers->HL);
			cpu_registers->B = tmp_c;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD B, (HL)");
#endif
			break;
		case 0x47:
			// LD B, A
			cpu_registers->B = cpu_registers->A;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD B, A");
#endif
			break;
		case 0x48:
			// LD C, B
			cpu_registers->C = cpu_registers->B;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD C, B");
#endif
			break;
		case 0x49:
			// LD C, C
			cpu_registers->C = cpu_registers->C;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD C, C");
#endif
			break;
		case 0x4A:
			// LD C, D
			cpu_registers->C = cpu_registers->D;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD C, D");
#endif
			break;
		case 0x4B:
			// LD C, E
			cpu_registers->C = cpu_registers->E;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD C, E");
#endif
			break;
		case 0x4C:
			// LD C, H
			cpu_registers->C = cpu_registers->H;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD C, H");
#endif
			break;
		case 0x4D:
			// LD C, L
			cpu_registers->C = cpu_registers->L;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD C, L");
#endif
			break;
		case 0x4E:
			// LD C, (HL)
			tmp_c = memory_read8(cpu_registers->HL);
			cpu_registers->C = tmp_c;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD C, (HL)");
#endif
			break;
		case 0x4F:
			// LD C, A
			cpu_registers->C = cpu_registers->A;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD C, A");
#endif
			break;

		case 0x50:
			// LD D, B
			cpu_registers->D = cpu_registers->B;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD D, B");
#endif
			break;
		case 0x51:
			// LD D, C
			cpu_registers->D = cpu_registers->C;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD D, C");
#endif
			break;
		case 0x52:
			// LD D, D
			cpu_registers->D = cpu_registers->D;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD D, D");
#endif
			break;
		case 0x53:
			// LD D, E
			cpu_registers->D = cpu_registers->E;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD D, E");
#endif
			break;
		case 0x54:
			// LD D, H
			cpu_registers->D = cpu_registers->H;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD D, H");
#endif
			break;
		case 0x55:
			// LD D, L
			cpu_registers->D = cpu_registers->L;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD D, L");
#endif
			break;
		case 0x56:
			// LD D, (HL)
			tmp_c = memory_read8(cpu_registers->HL);
			cpu_registers->D = tmp_c;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD D, (HL)");
#endif
			break;
		case 0x57:
			// LD D, A
			cpu_registers->D = cpu_registers->A;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD D, A");
#endif
			break;
		case 0x58:
			// LD E, B
			cpu_registers->E = cpu_registers->B;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD E, B");
#endif
			break;
		case 0x59:
			// LD E, C
			cpu_registers->E = cpu_registers->C;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD E, C");
#endif
			break;
		case 0x5A:
			// LD E, D
			cpu_registers->E = cpu_registers->D;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD E, D");
#endif
			break;
		case 0x5B:
			// LD E, E
			cpu_registers->E = cpu_registers->E;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD E, E");
#endif
			break;
		case 0x5C:
			// LD E, H
			cpu_registers->E = cpu_registers->H;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD E, H");
#endif
			break;
		case 0x5D:
			// LD E, L
			cpu_registers->E = cpu_registers->L;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD E, L");
#endif
			break;
		case 0x5E:
			// LD E, (HL)
			tmp_c = memory_read8(cpu_registers->HL);
			cpu_registers->E = tmp_c;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD E, (HL)");
#endif
			break;
		case 0x5F:
			// LD E, A
			cpu_registers->E = cpu_registers->A;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD E, A");
#endif
			break;
		
		case 0x60:
			// LD H, B
			cpu_registers->H = cpu_registers->B;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD H, B");
#endif
			break;
		case 0x61:
			// LD H, C
			cpu_registers->H = cpu_registers->C;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD H, C");
#endif
			break;
		case 0x62:
			// LD H, D
			cpu_registers->H = cpu_registers->D;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD H, D");
#endif
			break;
		case 0x63:
			// LD H, E
			cpu_registers->H = cpu_registers->E;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD H, E");
#endif
			break;
		case 0x64:
			// LD H, H
			cpu_registers->H = cpu_registers->H;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD H, H");
#endif
			break;
		case 0x65:
			// LD H, L
			cpu_registers->H = cpu_registers->L;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD H, L");
#endif
			break;
		case 0x66:
			// LD H, (HL)
			tmp_c = memory_read8(cpu_registers->HL);
			cpu_registers->H = tmp_c;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD H, (HL)");
#endif
			break;
		case 0x67:
			// LD H, A
			cpu_registers->H = cpu_registers->A;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD H, A");
#endif
			break;
		case 0x68:
			// LD L, B
			cpu_registers->L = cpu_registers->B;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD L, B");
#endif
			break;
		case 0x69:
			// LD L, C
			cpu_registers->L = cpu_registers->C;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD L, C");
#endif
			break;
		case 0x6A:
			// LD L, D
			cpu_registers->L = cpu_registers->D;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD L, D");
#endif
			break;
		case 0x6B:
			// LD L, E
			cpu_registers->L = cpu_registers->E;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD L, E");
#endif
			break;
		case 0x6C:
			// LD L, H
			cpu_registers->L = cpu_registers->H;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD L, H");
#endif
			break;
		case 0x6D:
			// LD L, L
			cpu_registers->L = cpu_registers->L;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD L, L");
#endif
			break;
		case 0x6E:
			// LD L, (HL)
			tmp_c = memory_read8(cpu_registers->HL);
			cpu_registers->L = tmp_c;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD L, (HL)");
#endif
			break;
		case 0x6F:
			// LD L, A
			cpu_registers->L = cpu_registers->A;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD L, A");
#endif
			break;
		
		case 0x70:
			// LD (HL), B
			memory_write8(cpu_registers->HL, cpu_registers->B);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD (HL), B");
#endif
			break;
		case 0x71:
			// LD (HL), C
			memory_write8(cpu_registers->HL, cpu_registers->C);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD (HL), C");
#endif
			break;
		case 0x72:
			// LD (HL), D
			memory_write8(cpu_registers->HL, cpu_registers->D);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD (HL), D");
#endif
			break;
		case 0x73:
			// LD (HL), E
			memory_write8(cpu_registers->HL, cpu_registers->E);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD (HL), E");
#endif
			break;
		case 0x74:
			// LD (HL), H
			memory_write8(cpu_registers->HL, cpu_registers->H);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD (HL), H");
#endif
			break;
		case 0x75:
			// LD (HL), L
			memory_write8(cpu_registers->HL, cpu_registers->L);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD (HL), L");
#endif
			break;
		case 0x76:
			// HALT
			// TODO: Implement logic
			cpu_state.halt = 1;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "HALT");
#endif
			break;
		case 0x77:
			// LD (HL), A
			memory_write8(cpu_registers->HL, cpu_registers->A);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD (HL), A");
#endif
			break;
		case 0x78:
			// LD A, B
			cpu_registers->A = cpu_registers->B;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD A, B");
#endif
			break;
		case 0x79:
			// LD A, C
			cpu_registers->A = cpu_registers->C;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD A, C");
#endif
			break;
		case 0x7A:
			// LD A, D
			cpu_registers->A = cpu_registers->D;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD A, D");
#endif
			break;
		case 0x7B:
			// LD A, E
			cpu_registers->A = cpu_registers->E;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD A, E");
#endif
			break;
		case 0x7C:
			// LD A, H
			cpu_registers->A = cpu_registers->H;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD A, H");
#endif
			break;
		case 0x7D:
			// LD A, L
			cpu_registers->A = cpu_registers->L;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD A, L");
#endif
			break;
		case 0x7E:
			// LD A, (HL)
			cpu_registers->A = memory_read8(cpu_registers->HL);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD A, (HL)");
#endif
			break;
		case 0x7F:
			// LD A, A
			cpu_registers->A = cpu_registers->A;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD A, A");
#endif
			break;
		
		case 0x86:
			// ADD A, (HL)
			cpu_registers->A+=(signed)memory_read8(cpu_registers->HL);
			cpu_registers->FLAG = (cpu_registers->A ? 0 : 1<<Z_FLAG);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "ADD A, (HL)");
#endif
			break;
		
		case 0x90:
			// SUB B
			// Algorithm from: https://github.com/drhelius/Gearboy/blob/4867b81c27d9b1144f077a20c6e2003ba21bd9a2/src/Processor_inline.h
			tmp_c = cpu_registers->A - cpu_registers->B;
			cpu_registers->A = tmp_c;
			
			// Z_FLAG and N_FLAG
			cpu_registers->FLAG = ((tmp_c ? 1 : 0)<<Z_FLAG) |
					 (1<<N_FLAG);
			
			// C_FLAG
			if(tmp_c & 0x100)
				cpu_registers->FLAG |= (1 << C_FLAG);
			
			// H_FLAG
			if(tmp_c & 0x10)
				cpu_registers->FLAG |= (1 << H_FLAG);
					
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "SUB B");
#endif			
			break;
		
		case 0xA0:
			// AND B
			cpu_registers->A &= cpu_registers->B;
			cpu_registers->FLAG = FLAG_COMPUTE_AND(cpu_registers->A);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "AND B");
#endif
			break;
		case 0xA1:
			// AND C
			cpu_registers->A &= cpu_registers->C;
			cpu_registers->FLAG = FLAG_COMPUTE_AND(cpu_registers->A);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "AND C");
#endif
			break;
		case 0xA2:
			// AND D
			cpu_registers->A &= cpu_registers->D;
			cpu_registers->FLAG = FLAG_COMPUTE_AND(cpu_registers->A);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "AND D");
#endif
			break;
		case 0xA3:
			// AND E
			cpu_registers->A &= cpu_registers->E;
			cpu_registers->FLAG = FLAG_COMPUTE_AND(cpu_registers->A);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "AND E");
#endif
			break;
		case 0xA4:
			// AND H
			cpu_registers->A &= cpu_registers->H;
			cpu_registers->FLAG = FLAG_COMPUTE_AND(cpu_registers->A);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "AND H");
#endif
			break;
		case 0xA5:
			// AND L
			cpu_registers->A &= cpu_registers->L;
			cpu_registers->FLAG = FLAG_COMPUTE_AND(cpu_registers->A);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "AND L");
#endif
			break;
		case 0xA6:
			// AND (HL)
			tmp_c = memory_read8(cpu_registers->HL);
			cpu_registers->A &= tmp_c;
			cpu_registers->FLAG = FLAG_COMPUTE_AND(cpu_registers->A);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "AND (HL)");
#endif
			break;
		case 0xA7:
			// AND A
			cpu_registers->A &= cpu_registers->A;
			cpu_registers->FLAG = FLAG_COMPUTE_AND(cpu_registers->A);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "AND A");
#endif
			break;
		case 0xA8:
			// XOR B
			cpu_registers->A ^= cpu_registers->B;
			cpu_registers->FLAG = (cpu_registers->A ? 0 : 1<<Z_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "XOR B");
#endif
			break;
		case 0xA9:
			// XOR C
			cpu_registers->A ^= cpu_registers->C;
			cpu_registers->FLAG = (cpu_registers->A ? 0 : 1<<Z_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "XOR C");
#endif
			break;
		case 0xAA:
			// XOR D
			cpu_registers->A ^= cpu_registers->D;
			cpu_registers->FLAG = (cpu_registers->A ? 0 : 1<<Z_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "XOR D");
#endif
			break;
		case 0xAB:
			// XOR E
			cpu_registers->A ^= cpu_registers->E;
			cpu_registers->FLAG = (cpu_registers->A ? 0 : 1<<Z_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "XOR E");
#endif
			break;
		case 0xAC:
			// XOR H
			cpu_registers->A ^= cpu_registers->H;
			cpu_registers->FLAG = (cpu_registers->A ? 0 : 1<<Z_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "XOR H");
#endif
			break;
		case 0xAD:
			// XOR L
			cpu_registers->A ^= cpu_registers->L;
			cpu_registers->FLAG = (cpu_registers->A ? 0 : 1<<Z_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "XOR L");
#endif
			break;
		case 0xAE:
			// XOR (HL)
			cpu_registers->A ^= memory_read16(cpu_registers->HL);
			cpu_registers->FLAG = (cpu_registers->A ? 0 : 1<<Z_FLAG);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "XOR (HL)");
#endif
			break;
		case 0xAF:
			// XOR A
			cpu_registers->A ^= cpu_registers->A;
			cpu_registers->FLAG = (cpu_registers->A ? 0 : 1<<Z_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "XOR A");
#endif
			break;
		
		case 0xB0:
			// OR B
			cpu_registers->A |= cpu_registers->B;
			cpu_registers->FLAG = (cpu_registers->A ? 0 : 1<<Z_FLAG);
			
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "OR B");
#endif
			break;
		case 0xB1:
			// OR B
			cpu_registers->A |= cpu_registers->B;
			cpu_registers->FLAG = (cpu_registers->A ? 0 : 1<<Z_FLAG);
			
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "OR B");
#endif
			break;
		case 0xB2:
			// OR D
			cpu_registers->A |= cpu_registers->D;
			cpu_registers->FLAG = (cpu_registers->A ? 0 : 1<<Z_FLAG);
			
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "OR D");
#endif
			break;
		case 0xB3:
			// OR E
			cpu_registers->A |= cpu_registers->E;
			cpu_registers->FLAG = (cpu_registers->A ? 0 : 1<<Z_FLAG);
			
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "OR E");
#endif
			break;
		case 0xB4:
			// OR H
			cpu_registers->A |= cpu_registers->H;
			cpu_registers->FLAG = (cpu_registers->A ? 0 : 1<<Z_FLAG);
			
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "OR H");
#endif
			break;
		case 0xB5:
			// OR L
			cpu_registers->A |= cpu_registers->L;
			cpu_registers->FLAG = (cpu_registers->A ? 0 : 1<<Z_FLAG);
			
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "OR L");
#endif
			break;
		case 0xB6:
			// OR (HL)
			cpu_registers->A |= memory_read8(cpu_registers->HL);
			cpu_registers->FLAG = (cpu_registers->A ? 0 : 1<<Z_FLAG);
			
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "OR (HL)");
#endif
			break;
		case 0xB7:
			// OR A
			cpu_registers->A |= cpu_registers->A;
			cpu_registers->FLAG = (cpu_registers->A ? 0 : 1<<Z_FLAG);
			
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "OR A");
#endif
			break;
		case 0xBE:
			// CP (HL)
			tmp_c = memory_read8(cpu_registers->HL);
			do_cp(tmp_c);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "CP (HL)");
#endif
			break;
		
		case 0xC1:
			// POP BC
			cpu_registers->BC = memory_read16(cpu_registers->SP);
			cpu_registers->SP += 2;
			cycles = 12;
#ifdef DISASSEMBLE
			sprintf(disassembly, "POP BC $%04x", cpu_registers->SP);
#endif
			break;
		case 0xC3:
			// JP nn
			cpu_registers->PC = memory_read16(cpu_registers->PC);
			cycles = 12;
#ifdef DISASSEMBLE
			sprintf(disassembly, "JP $%04x", cpu_registers->PC);
#endif
			break;
		case 0xC9:
			// RET
			// POP Ret Addr
			cpu_registers->PC = memory_read16(cpu_registers->SP);
			cpu_registers->SP += 2;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RET: $%04x", cpu_registers->PC);
#endif
			break;
		case 0xC5:
			// PUSH BC
			cpu_registers->SP -= 2;
			memory_write16(cpu_registers->SP, cpu_registers->BC);
			cycles = 16;
#ifdef DISASSEMBLE
			sprintf(disassembly, "PUSH BC $%04x", cpu_registers->SP);
#endif
			break;
		case 0xC7:
			// RST 0x00
			cpu_registers->SP -= 2;
			memory_write16(cpu_registers->SP, cpu_registers->PC);
			cpu_registers->PC = 0x00;
			cycles = 32;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RST $00");
#endif
			break;
		case 0xCB:
			// Prefixed opcode
			opcode = memory_read8(cpu_registers->PC++);
			cycles = parse_prefixed_opcode(opcode);
			break;
		case 0xCD:
			// Call nn
			tmp_s = memory_read16(cpu_registers->PC);
			cpu_registers->PC += 2;
			
			// PUSH next addr
			cpu_registers->SP -= 2;
			memory_write16(cpu_registers->SP, cpu_registers->PC);
			// Jump
			cpu_registers->PC = tmp_s;
			
			cycles = 12;
#ifdef DISASSEMBLE
			sprintf(disassembly, "CALL $%04x", tmp_s);
#endif
			break;
		case 0xCF:
			// RST 0x08
			cpu_registers->SP -= 2;
			memory_write16(cpu_registers->SP, cpu_registers->PC);
			cpu_registers->PC = 0x08;
			cycles = 32;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RST $08");
#endif
			break;


		case 0xD7:
			// RST 0x10
			cpu_registers->SP -= 2;
			memory_write16(cpu_registers->SP, cpu_registers->PC);
			cpu_registers->PC = 0x10;
			cycles = 32;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RST $10");
#endif
			break;
		case 0xD9:
			// RETI
			// POP Ret Addr
			cpu_registers->PC = memory_read16(cpu_registers->SP);
			cpu_registers->SP += 2;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RETI: $%04x", cpu_registers->PC);
#endif
			break;
		case 0xDF:
			// RST 0x18
			cpu_registers->SP -= 2;
			memory_write16(cpu_registers->SP, cpu_registers->PC);
			cpu_registers->PC = 0x18;
			cycles = 32;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RST $18");
#endif
			break;

		case 0xE0:
			// LD ($FF00+n), A
			tmp_c = memory_read8(cpu_registers->PC++);
			memory_write8(0xFF00 + tmp_c, cpu_registers->A);
			cycles = 12;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD ($ff00 + $%02x), A", tmp_c);
#endif
			break;
		case 0xE2:
			// LD ($FF00 + C), A
			memory_write8(0xFF00 + cpu_registers->C, cpu_registers->A);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD ($ff00 + C), A");
#endif
			break;
		case 0xE6:
			// AND #
			tmp_c = memory_read8(cpu_registers->PC++);
			cpu_registers->A &= tmp_c;
			cpu_registers->FLAG = FLAG_COMPUTE_AND(cpu_registers->A);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "AND $%02x", tmp_c);
#endif
			break;
		case 0xE7:
			// RST 0x20
			cpu_registers->SP -= 2;
			memory_write16(cpu_registers->SP, cpu_registers->PC);
			cpu_registers->PC = 0x20;
			cycles = 32;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RST $20");
#endif
			break;
		case 0xEA:
			// LD (nn), A
			tmp_s = memory_read16(cpu_registers->PC);
			cpu_registers->PC += 2;
			
			memory_write16(tmp_s, cpu_registers->A);
			
			cycles = 16;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD ($%04x), A", tmp_s);
#endif
			break;
		case 0xEF:
			// RST 0x28
			cpu_registers->SP -= 2;
			memory_write16(cpu_registers->SP, cpu_registers->PC);
			cpu_registers->PC = 0x28;
			cycles = 32;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RST $28");
#endif
			break;
		
		case 0xF0:
			// LD A,($FF00+n)
			tmp_c = memory_read8(cpu_registers->PC++);
			cpu_registers->A = memory_read8(0xFF00 + tmp_c);
			
			cycles = 12;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD A,($FF00 + $%02x)", tmp_c);
#endif
			break;
		case 0xF3:
			// DI - Disable Interrupts
			cpu_state.ime = 0;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "DI");
#endif
			break;
		case 0xF7:
			// RST 0x30
			cpu_registers->SP -= 2;
			memory_write16(cpu_registers->SP, cpu_registers->PC);
			cpu_registers->PC = 0x30;
			cycles = 32;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RST $30");
#endif
			break;
		case 0xFB:
			// EI
			cpu_state.ime = 1;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "EI");
#endif
		case 0xFE:
			// CP n
			tmp_c = memory_read8(cpu_registers->PC++);
			do_cp(tmp_c);
#ifdef DISASSEMBLE
			sprintf(disassembly, "CP $%02x", tmp_c);
#endif		
			cycles = 8;
			break;
		case 0xFF:
			// RST 0x38
			cpu_registers->SP -= 2;
			memory_write16(cpu_registers->SP, cpu_registers->PC);
			cpu_registers->PC = 0x38;
			cycles = 32;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RST $38");
#endif
			break;
		
		default:
#ifdef DISASSEMBLE
			sprintf(disassembly, "Unimplemented: 0x%02x", opcode);
#endif
			cycles = 0;
	}
	
	return cycles;
}

static unsigned char parse_prefixed_opcode(unsigned char opcode) {
	unsigned char cycles;
	unsigned char tmp_c;
	unsigned short tmp_s;
	
	switch(opcode)
	{
		case 0x10:
			// RL B
			// Get the far left bit
			// This sets the value to the correct position for C_FLAG
			// Z_FLAG == 0x7 -> 1<<0x7 == 128
			tmp_c = (cpu_registers->B & 128) >> (Z_FLAG - C_FLAG);
			
			// Shift all bits left + add carry bit
			cpu_registers->B = (cpu_registers->B << 1) | GET_BIT(cpu_registers->FLAG, C_FLAG);
			
			// Z-Dynamic, N-Reset, H-Reset, C-Contains old bit 7 data
			cpu_registers->FLAG = (((!cpu_registers->B) << Z_FLAG) | tmp_c);
			
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RL B");
#endif
			break;
		case 0x11:
			// RL C
			// See 0xCB 0x10 for more information on the RL opcode
			tmp_c = (cpu_registers->C & 128) >> (Z_FLAG - C_FLAG);
			
			cpu_registers->C = (cpu_registers->C << 1) | GET_BIT(cpu_registers->FLAG, C_FLAG);
			
			cpu_registers->FLAG = (((!cpu_registers->C) << Z_FLAG) | tmp_c);
			
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RL C");
#endif
			break;
		case 0x12:
			// RL D
			// See 0xCB 0x10 for more information on the RL opcode
			tmp_c = (cpu_registers->D & 128) >> (Z_FLAG - C_FLAG);
			
			cpu_registers->D = (cpu_registers->D << 1) | GET_BIT(cpu_registers->FLAG, C_FLAG);
			
			cpu_registers->FLAG = (((!cpu_registers->D) << Z_FLAG) | tmp_c);
			
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RL D");
#endif
			break;
		case 0x13:
			// RL E
			// See 0xCB 0x10 for more information on the RL opcode
			tmp_c = (cpu_registers->E & 128) >> (Z_FLAG - C_FLAG);
			
			cpu_registers->E = (cpu_registers->E << 1) | GET_BIT(cpu_registers->FLAG, C_FLAG);
			
			cpu_registers->FLAG = (((!cpu_registers->E) << Z_FLAG) | tmp_c);
			
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RL E");
#endif
			break;
		case 0x14:
			// RL H
			// See 0xCB 0x10 for more information on the RL opcode
			tmp_c = (cpu_registers->H & 128) >> (Z_FLAG - C_FLAG);
			
			cpu_registers->H = (cpu_registers->H << 1) | GET_BIT(cpu_registers->FLAG, C_FLAG);
			
			cpu_registers->FLAG = (((!cpu_registers->H) << Z_FLAG) | tmp_c);
			
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RL C");
#endif
			break;
		case 0x15:
			// RL L
			// See 0xCB 0x10 for more information on the RL opcode
			tmp_c = (cpu_registers->L & 128) >> (Z_FLAG - C_FLAG);
			
			cpu_registers->L = (cpu_registers->L << 1) | GET_BIT(cpu_registers->FLAG, C_FLAG);
			
			cpu_registers->FLAG = (((!cpu_registers->L) << Z_FLAG) | tmp_c);
			
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RL L");
#endif
			break;
		case 0x17:
			// RL A
			// See 0xCB 0x10 for more information on the RL opcode
			tmp_c = (cpu_registers->A & 128) >> (Z_FLAG - C_FLAG);
			
			cpu_registers->A = (cpu_registers->A << 1) | GET_BIT(cpu_registers->FLAG, C_FLAG);
			
			cpu_registers->FLAG = (((!cpu_registers->A) << Z_FLAG) | tmp_c);
			
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RL A");
#endif
			break;
		
		case 0x30:
			// SWAP B
			tmp_c = cpu_registers->B;
			cpu_registers->B = ((tmp_c & 0xF)<<4 | (tmp_c & 0xF0)>>4);
			cpu_registers->FLAG = (cpu_registers->B ? 0 : 1<<Z_FLAG);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "SWAP B");
#endif
			break;
		case 0x31:
			// SWAP C
			tmp_c = cpu_registers->C;
			cpu_registers->C = ((tmp_c & 0xF)<<4 | (tmp_c & 0xF0)>>4);
			cpu_registers->FLAG = (cpu_registers->C ? 0 : 1<<Z_FLAG);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "SWAP C");
#endif
			break;
		case 0x32:
			// SWAP D
			tmp_c = cpu_registers->D;
			cpu_registers->D = ((tmp_c & 0xF)<<4 | (tmp_c & 0xF0)>>4);
			cpu_registers->FLAG = (cpu_registers->D ? 0 : 1<<Z_FLAG);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "SWAP D");
#endif
			break;
		case 0x33:
			// SWAP E
			tmp_c = cpu_registers->E;
			cpu_registers->E = ((tmp_c & 0xF)<<4 | (tmp_c & 0xF0)>>4);
			cpu_registers->FLAG = (cpu_registers->E ? 0 : 1<<Z_FLAG);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "SWAP E");
#endif
			break;
		case 0x34:
			// SWAP H
			tmp_c = cpu_registers->H;
			cpu_registers->H = ((tmp_c & 0xF)<<4 | (tmp_c & 0xF0)>>4);
			cpu_registers->FLAG = (cpu_registers->H ? 0 : 1<<Z_FLAG);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "SWAP H");
#endif
			break;
		case 0x35:
			// SWAP L
			tmp_c = cpu_registers->L;
			cpu_registers->L = ((tmp_c & 0xF)<<4 | (tmp_c & 0xF0)>>4);
			cpu_registers->FLAG = (cpu_registers->L ? 0 : 1<<Z_FLAG);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "SWAP L");
#endif
			break;
		case 0x36:
			// SWAP (HL)
			tmp_c = memory_read8(cpu_registers->HL);
			tmp_c = ((tmp_c & 0xF)<<4 | (tmp_c & 0xF0)>>4);
			memory_write8(cpu_registers->HL, tmp_c);
			cpu_registers->FLAG = (tmp_c ? 0 : 1<<Z_FLAG);
			cycles = 8;
#ifdef disassemble
			sprintf(disassembly, "SWAP (HL)");
#endif
			break;
		case 0x37:
			// SWAP A
			tmp_c = cpu_registers->A;
			cpu_registers->A = ((tmp_c & 0xF)<<4 | (tmp_c & 0xF0)>>4);
			cpu_registers->FLAG = (cpu_registers->A ? 0 : 1<<Z_FLAG);
			cycles = 8;
#ifdef disassemble
			sprintf(disassembly, "SWAP A");
#endif
			break;
		
		case 0x78:
			// BIT 7, B
			tmp_c = (GET_BIT(cpu_registers->B, 7) << Z_FLAG);
			cpu_registers->FLAG = tmp_c & FLAG_PRECOMPUTE_BIT;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "BIT 7, B");
#endif
			break;
		case 0x79:
			// BIT 7, C
			tmp_c = (GET_BIT(cpu_registers->C, 7) << Z_FLAG);
			cpu_registers->FLAG = tmp_c & FLAG_PRECOMPUTE_BIT;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "BIT 7, C");
#endif
			break;
		case 0x7A:
			// BIT 7, D
			tmp_c = (GET_BIT(cpu_registers->D, 7) << Z_FLAG);
			cpu_registers->FLAG = tmp_c & FLAG_PRECOMPUTE_BIT;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "BIT 7, D");
#endif
			break;
		case 0x7B:
			// BIT 7, E
			tmp_c = (GET_BIT(cpu_registers->E, 7) << Z_FLAG);
			cpu_registers->FLAG = tmp_c & FLAG_PRECOMPUTE_BIT;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "BIT 7, E");
#endif
			break;
		case 0x7C:
			// BIT 7, H
			tmp_c = (GET_BIT(cpu_registers->H, 7) << Z_FLAG);
			cpu_registers->FLAG = tmp_c & FLAG_PRECOMPUTE_BIT;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "BIT 7, H");
#endif
			break;
		case 0x7D:
			// BIT 7, (HL)
			tmp_c = memory_read8(cpu_registers->HL);
			tmp_c = (GET_BIT(tmp_c, 7) << Z_FLAG);
			cpu_registers->FLAG = tmp_c & FLAG_PRECOMPUTE_BIT;
			cycles = 16;
#ifdef DISASSEMBLE
			sprintf(disassembly, "BIT 7, (HL)");
#endif
			break;
		case 0x7F:
			// BIT 7, A
			tmp_c = (GET_BIT(cpu_registers->A, 7) << Z_FLAG);
			cpu_registers->FLAG = tmp_c & FLAG_PRECOMPUTE_BIT;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "BIT 7, A");
#endif
			break;

		default:
#ifdef DISASSEMBLE
			sprintf(disassembly, "Unimplemented: 0xCB 0x%02x", opcode);
#endif

			cycles = 0;
	}
	
	return cycles;
}

static void cpu_rom_reset() {
	/**
	When loading the ROM, set the following values

	   AF=$01B0
	   BC=$0013
	   DE=$00D8
	   HL=$014D
	   Stack Pointer=$FFFE
	   [$FF05] = $00   ; TIMA
	   [$FF06] = $00   ; TMA
	   [$FF07] = $00   ; TAC
	   [$FF10] = $80   ; NR10
	   [$FF11] = $BF   ; NR11
	   [$FF12] = $F3   ; NR12
	   [$FF14] = $BF   ; NR14
	   [$FF16] = $3F   ; NR21
	   [$FF17] = $00   ; NR22
	   [$FF19] = $BF   ; NR24
	   [$FF1A] = $7F   ; NR30
	   [$FF1B] = $FF   ; NR31
	   [$FF1C] = $9F   ; NR32
	   [$FF1E] = $BF   ; NR33
	   [$FF20] = $FF   ; NR41
	   [$FF21] = $00   ; NR42
	   [$FF22] = $00   ; NR43
	   [$FF23] = $BF   ; NR30
	   [$FF24] = $77   ; NR50
	   [$FF25] = $F3   ; NR51
	   [$FF26] = $F1-GB, $F0-SGB ; NR52
	   [$FF40] = $91   ; LCDC
	   [$FF42] = $00   ; SCY
	   [$FF43] = $00   ; SCX
	   [$FF45] = $00   ; LYC
	   [$FF47] = $FC   ; BGP
	   [$FF48] = $FF   ; OBP0
	   [$FF49] = $FF   ; OBP1
	   [$FF4A] = $00   ; WY
	   [$FF4B] = $00   ; WX
	   [$FFFF] = $00   ; IE
	*/
	
	cpu_registers->AF = 0x01B0;
	cpu_registers->BC = 0x0013;
	cpu_registers->DE = 0x00D8;
	cpu_registers->HL = 0x014D;
	cpu_registers->SP = 0xFFFE;
	
	memory_write8(0xFF05, 0x00);
	memory_write8(0xFF06, 0x00);
	memory_write8(0xFF07, 0x00);
	memory_write8(0xFF10, 0x80);
	memory_write8(0xFF11, 0xBF);
	memory_write8(0xFF12, 0xF3);	
	memory_write8(0xFF14, 0xBF);
	memory_write8(0xFF16, 0x3F);
	memory_write8(0xFF17, 0x00);
	memory_write8(0xFF19, 0xBF);
	memory_write8(0xFF1A, 0x7F);
	memory_write8(0xFF1B, 0xFF);
	memory_write8(0xFF1C, 0x9F);
	memory_write8(0xFF1E, 0xBF);
	memory_write8(0xFF20, 0xFF);
	memory_write8(0xFF21, 0x00);
	memory_write8(0xFF22, 0x00);
	memory_write8(0xFF23, 0xBF);
	memory_write8(0xFF24, 0x77);
	memory_write8(0xFF25, 0xF3);
	memory_write8(0xFF26, 0x00); // TODO: This value depends on system
	memory_write8(0xFF40, 0x91);
	memory_write8(0xFF42, 0x00);
	memory_write8(0xFF43, 0x00);
	memory_write8(0xFF45, 0x00);
	memory_write8(0xFF47, 0xFC);
	memory_write8(0xFF48, 0xFF);
	memory_write8(0xFF49, 0xFF);
	memory_write8(0xFF4A, 0x00);
	memory_write8(0xFF4B, 0x00);
	memory_write8(0xFFFF, 0x00);
	
	rom_set_preamble();
}

/**
	This is odd logic, so abstracted incase needed to change
*/
static void do_cp(unsigned char val) {
	/**
	Z - Set if result is zero. (Set if A = n.)
	N - Set.
	H - Set if no borrow from bit 4.
	C - Set for no borrow. (Set if A < n.)
	*/
	
	// Set half-carry flag
	cpu_registers->FLAG = (!(((cpu_registers->A & 0xF) - (val)) & 0xF)) <<H_FLAG;
	cpu_registers->FLAG |= 1 << N_FLAG;
	
	/**
	Remember:
		tmp_c is unsigned
		If tmp_c is signed, 129 wraps and makes neg.
	*/
	val = cpu_registers->A - val;
	if(val > 128)
		cpu_registers->FLAG |= (1 << C_FLAG);
	else if(!val)
		cpu_registers->FLAG |= (1 << Z_FLAG);
}
