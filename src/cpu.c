#include "cpu.h"
#include "memory.h"

/**
Global Variables
*/
unsigned char cpu_running;

/**
Static Functions
*/
static unsigned char parse_opcode(unsigned char opcode);
static unsigned char parse_prefixed_opcode(unsigned char opcode);

static void do_cp(unsigned char val);

/**
Static Variables
*/
static struct registers cpu_registers;
static char * interrupt_register;

#ifdef DISASSEMBLE
static char disassembly[255];
#endif

/**
Functions
*/
void cpu_init() {
	cpu_reset();
}

void cpu_reset() {
	// Reset all registers
	cpu_registers = (struct registers){
		.AF = 0,
		.BC = 0,
		.DE = 0,
		.HL = 0,
		.PC = 0,
		.SP = 0
	};
	
	cpu_running = 1;
}

void cpu_dump_regs() {
	printf("A:  %02x\n", cpu_registers.A);
	printf("B:  %02x\n", cpu_registers.B);
	printf("C:  %02x\n", cpu_registers.C);
	printf("D:  %02x\n", cpu_registers.D);
	printf("E:  %02x\n", cpu_registers.E);
	printf("H:  %02x\n", cpu_registers.H);
	printf("L:  %02x\n", cpu_registers.L);
	printf("AF: %04x\n", cpu_registers.AF);
	printf("BC: %04x\n", cpu_registers.BC);
	printf("DE: %04x\n", cpu_registers.DE);
	printf("HL: %04x\n", cpu_registers.HL);
	printf("SP: %04x\n", cpu_registers.SP);
	printf("PC: %04x\n", cpu_registers.PC);
}

unsigned char cpu_step() {
	unsigned char byte;
#ifdef DISASSEMBLE
	unsigned char cycles;
	unsigned short pc_start;
	
	pc_start = cpu_registers.PC;
#endif
	if(cpu_registers.PC == 0xe9) {
		cpu_running = 0;
		return 0;
	}
	byte = memory_read8(cpu_registers.PC++);
#ifdef DISASSEMBLE
	memset(disassembly, 0, sizeof(disassembly));
	
	cycles = parse_opcode(byte);
	printf("$%04x %s\n", pc_start, disassembly);
	
	if(!cycles) exit(0);
	
	return cycles;
#else
	return parse_opcode(byte);
#endif
}

static unsigned char parse_opcode(unsigned char opcode) {
	unsigned char cycles;
	unsigned char tmp_c;
	unsigned short tmp_s;
	
	switch(opcode)
	{
		case 0x01:
			// LD BC, nn
			cpu_registers.BC = memory_read16(cpu_registers.PC);
			cpu_registers.PC += 2;
			cycles = 12;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD BC, $%04x", cpu_registers.BC);
#endif
			break;
		case 0x03:
			// INC BC
			cpu_registers.BC++;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "INC BC");
#endif
			break;
		case 0x04:
			// INC B
			cpu_registers.B++;
			cpu_registers.FLAG = FLAG_COMPUTE_INC(cpu_registers.B) | GET_BIT(cpu_registers.FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "INC B");
#endif
			break;
		case 0x05:
			// DEC B
			cpu_registers.B--;
			cpu_registers.FLAG = FLAG_COMPUTE_DEC(cpu_registers.B) | GET_BIT(cpu_registers.FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "DEC B");
#endif
			break;
		case 0x06:
			// LD B, n
			cpu_registers.B = memory_read8(cpu_registers.PC++);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD B, $%02x", cpu_registers.B);
#endif
			break;
		case 0x0A:
			// LD A, (BC)
			cpu_registers.A = memory_read8(cpu_registers.BC);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD A, (BC)");
#endif
			break;

		case 0x0C:
			// INC C
			cpu_registers.C++;
			cpu_registers.FLAG = FLAG_COMPUTE_INC(cpu_registers.C) | GET_BIT(cpu_registers.FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "INC C");
#endif
			break;
		case 0x0D:
			// DEC C
			cpu_registers.C--;
			cpu_registers.FLAG = FLAG_COMPUTE_DEC(cpu_registers.C) | GET_BIT(cpu_registers.FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "DEC C");
#endif
			break;
		case 0x0E:
			// LD C, n
			cpu_registers.C = memory_read8(cpu_registers.PC++);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD C, $%02x", cpu_registers.C);
#endif
			break;
		
		case 0x11:
			// LD DE, nn
			cpu_registers.DE = memory_read16(cpu_registers.PC);
			cpu_registers.PC += 2;
			cycles = 12;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD DE, $%04x", cpu_registers.DE);
#endif
			break;
		case 0x13:
			// INC DE
			cpu_registers.DE++;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "INC DE");
#endif
			break;
		case 0x14:
			// INC D
			cpu_registers.D++;
			cpu_registers.FLAG = FLAG_COMPUTE_INC(cpu_registers.D) | GET_BIT(cpu_registers.FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "INC D");
#endif
			break;
		case 0x15:
			// DEC D
			cpu_registers.D--;
			cpu_registers.FLAG = FLAG_COMPUTE_DEC(cpu_registers.D) | GET_BIT(cpu_registers.FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "DEC D");
#endif
			break;
		case 0x16:
			// LD D, n
			cpu_registers.D = memory_read8(cpu_registers.PC++);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD D, $%02x", cpu_registers.D);
#endif
			break;
		case 0x17:
			// RLA
			// See 0xCB 0x17 for more info on this algorithm
			tmp_c = (cpu_registers.A & 128) >> (Z_FLAG - C_FLAG);
			cpu_registers.A = (cpu_registers.A << 1) | GET_BIT(cpu_registers.FLAG, C_FLAG);
			cpu_registers.FLAG = tmp_c;
			
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RLA");
#endif
			break;
		case 0x18:
			// JR n
			tmp_c = memory_read8(cpu_registers.PC);
			cpu_registers.PC++;
			
			cpu_registers.PC += (signed char)tmp_c;
			
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "JR %d", tmp_c);
#endif
			break;
		case 0x1A:
			// LD A, (DE)
			cpu_registers.A = memory_read8(cpu_registers.DE);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD A, (DE)");
#endif
			break;
		case 0x1C:
			// INC E
			cpu_registers.E++;
			cpu_registers.FLAG = FLAG_COMPUTE_INC(cpu_registers.E) | GET_BIT(cpu_registers.FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "INC E");
#endif
			break;
		case 0x1D:
			// DEC E
			cpu_registers.E--;
			cpu_registers.FLAG = FLAG_COMPUTE_DEC(cpu_registers.E) | GET_BIT(cpu_registers.FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "DEC E");
#endif
			break;
		case 0x1E:
			// LD E, n
			cpu_registers.E = memory_read8(cpu_registers.PC++);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD E, $%02x", cpu_registers.E);
#endif
			break;
		
		case 0x20:
			// JR NZ, *
			tmp_c = memory_read16(cpu_registers.PC++);
			
			// Jump if Z-flag is reset
			if(!GET_BIT(cpu_registers.FLAG, Z_FLAG))
				cpu_registers.PC += (signed char)tmp_c;
			
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "JR NZ, %02d", tmp_c);
#endif
			break;
		case 0x21:
			// LD HL, nn
			cpu_registers.HL = memory_read16(cpu_registers.PC);
			cpu_registers.PC += 2;
			cycles = 12;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD HL, $%04x", cpu_registers.HL);
#endif
			break;
		case 0x22:
			// LD (HL+), A
			memory_write8(cpu_registers.HL++, cpu_registers.A);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD (HL+), A");
#endif
			break;
		case 0x23:
			// INC HL
			cpu_registers.HL++;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "INC HL");
#endif
			break;
		case 0x24:
			// INC H
			cpu_registers.H++;
			cpu_registers.FLAG = FLAG_COMPUTE_INC(cpu_registers.H) | GET_BIT(cpu_registers.FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "INC H");
#endif
			break;
		case 0x25:
			// DEC H
			cpu_registers.H--;
			cpu_registers.FLAG = FLAG_COMPUTE_DEC(cpu_registers.H) | GET_BIT(cpu_registers.FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "DEC H");
#endif
			break;
		case 0x26:
			// LD H, n
			cpu_registers.H = memory_read8(cpu_registers.PC++);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD H, $%02x", cpu_registers.H);
#endif
			break;
		case 0x28:
			// JR Z, *
			tmp_c = memory_read8(cpu_registers.PC++);
			
			// Jump if Z-flag is set
			if(GET_BIT(cpu_registers.FLAG, Z_FLAG))
				cpu_registers.PC += (signed char)tmp_c;
			
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "JR Z, %02d", tmp_c);
#endif
			break;
		case 0x2C:
			// INC L
			cpu_registers.L++;
			cpu_registers.FLAG = FLAG_COMPUTE_INC(cpu_registers.L) | GET_BIT(cpu_registers.FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "INC L");
#endif
			break;
		case 0x2D:
			// DEC L
			cpu_registers.L--;
			cpu_registers.FLAG = FLAG_COMPUTE_DEC(cpu_registers.L) | GET_BIT(cpu_registers.FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "DEC L");
#endif
			break;
		case 0x2E:
			// LD L, n
			cpu_registers.L = memory_read8(cpu_registers.PC++);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD L, $%02x", cpu_registers.L);
#endif
			break;
		
		case 0x30:
			// JR NC, *
			tmp_c = memory_read8(cpu_registers.PC++);
			
			// Jump if C-flag is reset
			if(!GET_BIT(cpu_registers.FLAG, C_FLAG))
				cpu_registers.PC += (signed char)tmp_c;
			
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "JR NC, %02d", tmp_c);
#endif
			break;
		case 0x31:
			// LD SP,$aabb
			cpu_registers.SP = memory_read16(cpu_registers.PC);
			cpu_registers.PC += 2;
			cycles = 12;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD SP, $%04x", cpu_registers.SP);
#endif
			break;
		case 0x32:
			// LD (HL-), A
			memory_write8(cpu_registers.HL--, cpu_registers.A);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD (HL-), A");
#endif
			break;
		case 0x33:
			// INC SP
			cpu_registers.SP++;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "INC SP");
#endif
			break;
		case 0x34:
			// INC (HL)
			tmp_c = memory_read8(cpu_registers.HL) + 1;
			memory_write8(cpu_registers.HL, tmp_c);
			cpu_registers.FLAG = FLAG_COMPUTE_INC(tmp_c) | GET_BIT(cpu_registers.FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "INC (HL)");
#endif
			break;
		case 0x35:
			// DEC (HL)
			tmp_c = memory_read8(cpu_registers.HL);
			memory_write8(cpu_registers.HL, --tmp_c);
			cpu_registers.FLAG = FLAG_COMPUTE_DEC(tmp_c) | GET_BIT(cpu_registers.FLAG, C_FLAG);
			cycles = 12;
#ifdef DISASSEMBLE
			sprintf(disassembly, "DEC (HL)");
#endif
			break;
		case 0x38:
			// JR C, *
			tmp_c = memory_read8(cpu_registers.PC++);
			
			// Jump if C-flag is set
			if(GET_BIT(cpu_registers.FLAG, C_FLAG))
				cpu_registers.PC += (signed char)tmp_c;
			
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "JR C, %02d", tmp_c);
#endif
			break;
		case 0x3C:
			// INC A
			cpu_registers.A++;
			cpu_registers.FLAG = FLAG_COMPUTE_INC(cpu_registers.A) | GET_BIT(cpu_registers.FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "INC A");
#endif
			break;
		case 0x3D:
			// DEC A
			cpu_registers.A--;
			cpu_registers.FLAG = FLAG_COMPUTE_DEC(cpu_registers.A) | GET_BIT(cpu_registers.FLAG, C_FLAG);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "DEC A");
#endif
			break;
		case 0x3E:
			// LD A, #
			cpu_registers.A = memory_read8(cpu_registers.PC++);
			
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD A, $%02x", cpu_registers.A);
#endif
			break;
		
		case 0x47:
			// LD B, A
			cpu_registers.B = cpu_registers.A;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD B, A");
#endif
			break;
		case 0x4F:
			// LD C, A
			cpu_registers.C = cpu_registers.A;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD C, A");
#endif
			break;
	
		case 0x57:
			// LD D, A
			cpu_registers.D = cpu_registers.A;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD D, A");
#endif
			break;
		case 0x5F:
			// LD E, A
			cpu_registers.E = cpu_registers.A;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD E, A");
#endif
			break;
		
		case 0x67:
			// LD H, A
			cpu_registers.H = cpu_registers.A;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD H, A");
#endif
			break;
		case 0x6F:
			// LD L, A
			cpu_registers.L = cpu_registers.A;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD L, A");
#endif
			break;

		case 0x77:
			// LD (HL), A
			memory_write8(cpu_registers.HL, cpu_registers.A);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD (HL), A");
#endif
			break;
		case 0x78:
			// LD A, B
			cpu_registers.A = cpu_registers.B;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD A, B");
#endif
			break;
		case 0x79:
			// LD A, C
			cpu_registers.A = cpu_registers.C;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD A, C");
#endif
			break;
		case 0x7A:
			// LD A, D
			cpu_registers.A = cpu_registers.D;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD A, D");
#endif
			break;
		case 0x7B:
			// LD A, E
			cpu_registers.A = cpu_registers.E;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD A, E");
#endif
			break;
		case 0x7C:
			// LD A, H
			cpu_registers.A = cpu_registers.H;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD A, H");
#endif
			break;
		case 0x7D:
			// LD A, L
			cpu_registers.A = cpu_registers.L;
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD A, L");
#endif
			break;
		case 0x7E:
			// LD A, (HL)
			cpu_registers.A = memory_read8(cpu_registers.HL);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD A, (HL)");
#endif
			break;
		
		case 0x90:
			// SUB B
			// Algorithm from: https://github.com/drhelius/Gearboy/blob/4867b81c27d9b1144f077a20c6e2003ba21bd9a2/src/Processor_inline.h
			tmp_c = cpu_registers.A - cpu_registers.B;
			cpu_registers.A = cpu_registers.A ^ cpu_registers.B ^ tmp_c;
			
			// Z_FLAG and N_FLAG
			cpu_registers.FLAG = ((tmp_c ? 1 : 0) << Z_FLAG) | (1 << N_FLAG);
			
			// C_FLAG
			if(tmp_c & 0x100)
				cpu_registers.FLAG |= (1 << C_FLAG);
			
			// H_FLAG
			if(tmp_c & 0x10)
				cpu_registers.FLAG |= (1 << H_FLAG);
					
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "SUB B");
#endif			
			break;
		
		case 0xA8:
			// XOR B
			cpu_registers.A ^= cpu_registers.B;
			cpu_registers.FLAG = (cpu_registers.A ? 0 : FLAG_PRECOMPUTE_XOR);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "XOR B");
#endif
			break;
		case 0xA9:
			// XOR C
			cpu_registers.A ^= cpu_registers.C;
			cpu_registers.FLAG = (cpu_registers.A ? 0 : FLAG_PRECOMPUTE_XOR);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "XOR C");
#endif
			break;
		case 0xAA:
			// XOR D
			cpu_registers.A ^= cpu_registers.D;
			cpu_registers.FLAG = (cpu_registers.A ? 0 : FLAG_PRECOMPUTE_XOR);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "XOR D");
#endif
			break;
		case 0xAB:
			// XOR E
			cpu_registers.A ^= cpu_registers.E;
			cpu_registers.FLAG = (cpu_registers.A ? 0 : FLAG_PRECOMPUTE_XOR);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "XOR E");
#endif
			break;
		case 0xAC:
			// XOR H
			cpu_registers.A ^= cpu_registers.H;
			cpu_registers.FLAG = (cpu_registers.A ? 0 : FLAG_PRECOMPUTE_XOR);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "XOR H");
#endif
			break;
		case 0xAD:
			// XOR L
			cpu_registers.A ^= cpu_registers.L;
			cpu_registers.FLAG = (cpu_registers.A ? 0 : FLAG_PRECOMPUTE_XOR);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "XOR L");
#endif
			break;
		case 0xAE:
			// XOR (HL)
			cpu_registers.A ^= memory_read16(cpu_registers.HL);
			cpu_registers.FLAG = (cpu_registers.A ? 0 : FLAG_PRECOMPUTE_XOR);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "XOR (HL)");
#endif
			break;
		case 0xAF:
			// XOR A
			cpu_registers.A ^= cpu_registers.A;
			cpu_registers.FLAG = (cpu_registers.A ? 0 : FLAG_PRECOMPUTE_XOR);
			cycles = 4;
#ifdef DISASSEMBLE
			sprintf(disassembly, "XOR A");
#endif
			break;
		
		case 0xBE:
			// CP (HL)
			tmp_c = memory_read8(cpu_registers.HL);
			do_cp(tmp_c);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "CP (HL)");
#endif
			break;
		
		case 0xC1:
			// POP BC
			cpu_registers.BC = memory_read16(cpu_registers.SP);
			cpu_registers.SP += 2;
			cycles = 12;
#ifdef DISASSEMBLE
			sprintf(disassembly, "POP BC $%04x", cpu_registers.SP);
#endif
			break;
		case 0xC9:
			// RET
			// POP Ret Addr
			cpu_registers.PC = memory_read16(cpu_registers.SP);
			cpu_registers.SP += 2;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RET: $%04x", cpu_registers.PC);
#endif
			break;
		case 0xC5:
			// PUSH BC
			cpu_registers.SP -= 2;
			memory_write16(cpu_registers.SP, cpu_registers.BC);
			cycles = 16;
#ifdef DISASSEMBLE
			sprintf(disassembly, "PUSH BC $%04x", cpu_registers.SP);
#endif
			break;
		case 0xCB:
			// Prefixed opcode
			opcode = memory_read8(cpu_registers.PC++);
			cycles = parse_prefixed_opcode(opcode);
			break;
		case 0xCD:
			// Call nn
			tmp_s = memory_read16(cpu_registers.PC);
			cpu_registers.PC += 2;
			
			// PUSH next addr
			cpu_registers.SP -= 2;
			memory_write16(cpu_registers.SP, cpu_registers.PC);
			printf("CALL -> $%04x\n", cpu_registers.PC);
			// Jump
			cpu_registers.PC = tmp_s;
			
			cycles = 12;
#ifdef DISASSEMBLE
			sprintf(disassembly, "CALL $%04x", tmp_s);
#endif
			break;
		
		case 0xE0:
			// LD ($FF00+n), A
			tmp_c = memory_read8(cpu_registers.PC++);
			memory_write8(0xFF00 + tmp_c, cpu_registers.A);
			cycles = 12;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD ($ff00 + $%02x), A", tmp_c);
#endif
			break;
		case 0xE2:
			// LD ($FF00 + C), A
			memory_write8(0xFF00 + cpu_registers.C, cpu_registers.A);
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD ($ff00 + C), A");
#endif
			break;
		case 0xEA:
			// LD (nn), A
			tmp_s = memory_read16(cpu_registers.PC);
			cpu_registers.PC += 2;
			
			memory_write16(tmp_s, cpu_registers.A);
			
			cycles = 16;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD ($%04x), A", tmp_s);
#endif
			break;
		
		case 0xF0:
			// LD A,($FF00+n)
			tmp_c = memory_read8(cpu_registers.PC);
			cpu_registers.PC++;
			
			cpu_registers.A = memory_read8(0xFF00 + tmp_c);
			
			cycles = 12;
#ifdef DISASSEMBLE
			sprintf(disassembly, "LD A,($FF00 + $%02x)", tmp_c);
#endif
			break;
		case 0xFE:
			// CP n
			tmp_c = memory_read8(cpu_registers.PC++);
			do_cp(tmp_c);
#ifdef DISASSEMBLE
			sprintf(disassembly, "CP $%02x (A: $%02x)", tmp_c, cpu_registers.A);
#endif		
			cycles = 8;
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
			tmp_c = (cpu_registers.B & 128) >> (Z_FLAG - C_FLAG);
			
			// Shift all bits left + add carry bit
			cpu_registers.B = (cpu_registers.B << 1) | GET_BIT(cpu_registers.FLAG, C_FLAG);
			
			// Z-Dynamic, N-Reset, H-Reset, C-Contains old bit 7 data
			cpu_registers.FLAG = (((!cpu_registers.B) << Z_FLAG) | tmp_c);
			
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RL B");
#endif
			break;
		case 0x11:
			// RL C
			// See 0xCB 0x10 for more information on the RL opcode
			tmp_c = (cpu_registers.C & 128) >> (Z_FLAG - C_FLAG);
			
			cpu_registers.C = (cpu_registers.C << 1) | GET_BIT(cpu_registers.FLAG, C_FLAG);
			
			cpu_registers.FLAG = (((!cpu_registers.C) << Z_FLAG) | tmp_c);
			
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RL C");
#endif
			break;
		case 0x12:
			// RL D
			// See 0xCB 0x10 for more information on the RL opcode
			tmp_c = (cpu_registers.D & 128) >> (Z_FLAG - C_FLAG);
			
			cpu_registers.D = (cpu_registers.D << 1) | GET_BIT(cpu_registers.FLAG, C_FLAG);
			
			cpu_registers.FLAG = (((!cpu_registers.D) << Z_FLAG) | tmp_c);
			
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RL D");
#endif
			break;
		case 0x13:
			// RL E
			// See 0xCB 0x10 for more information on the RL opcode
			tmp_c = (cpu_registers.E & 128) >> (Z_FLAG - C_FLAG);
			
			cpu_registers.E = (cpu_registers.E << 1) | GET_BIT(cpu_registers.FLAG, C_FLAG);
			
			cpu_registers.FLAG = (((!cpu_registers.E) << Z_FLAG) | tmp_c);
			
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RL E");
#endif
			break;
		case 0x14:
			// RL H
			// See 0xCB 0x10 for more information on the RL opcode
			tmp_c = (cpu_registers.H & 128) >> (Z_FLAG - C_FLAG);
			
			cpu_registers.H = (cpu_registers.H << 1) | GET_BIT(cpu_registers.FLAG, C_FLAG);
			
			cpu_registers.FLAG = (((!cpu_registers.H) << Z_FLAG) | tmp_c);
			
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RL C");
#endif
			break;
		case 0x15:
			// RL L
			// See 0xCB 0x10 for more information on the RL opcode
			tmp_c = (cpu_registers.L & 128) >> (Z_FLAG - C_FLAG);
			
			cpu_registers.L = (cpu_registers.L << 1) | GET_BIT(cpu_registers.FLAG, C_FLAG);
			
			cpu_registers.FLAG = (((!cpu_registers.L) << Z_FLAG) | tmp_c);
			
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RL L");
#endif
			break;
		case 0x17:
			// RL A
			// See 0xCB 0x10 for more information on the RL opcode
			tmp_c = (cpu_registers.A & 128) >> (Z_FLAG - C_FLAG);
			
			cpu_registers.A = (cpu_registers.A << 1) | GET_BIT(cpu_registers.FLAG, C_FLAG);
			
			cpu_registers.FLAG = (((!cpu_registers.A) << Z_FLAG) | tmp_c);
			
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "RL A");
#endif
			break;

		case 0x78:
			// BIT 7, B
			tmp_c = (GET_BIT(cpu_registers.B, 7) << Z_FLAG);
			cpu_registers.FLAG = tmp_c & FLAG_PRECOMPUTE_BIT;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "BIT 7, B");
#endif
			break;
		case 0x79:
			// BIT 7, C
			tmp_c = (GET_BIT(cpu_registers.C, 7) << Z_FLAG);
			cpu_registers.FLAG = tmp_c & FLAG_PRECOMPUTE_BIT;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "BIT 7, C");
#endif
			break;
		case 0x7A:
			// BIT 7, D
			tmp_c = (GET_BIT(cpu_registers.D, 7) << Z_FLAG);
			cpu_registers.FLAG = tmp_c & FLAG_PRECOMPUTE_BIT;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "BIT 7, D");
#endif
			break;
		case 0x7B:
			// BIT 7, E
			tmp_c = (GET_BIT(cpu_registers.E, 7) << Z_FLAG);
			cpu_registers.FLAG = tmp_c & FLAG_PRECOMPUTE_BIT;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "BIT 7, E");
#endif
			break;
		case 0x7C:
			// BIT 7, H
			tmp_c = (GET_BIT(cpu_registers.H, 7) << Z_FLAG);
			cpu_registers.FLAG = tmp_c & FLAG_PRECOMPUTE_BIT;
			cycles = 8;
#ifdef DISASSEMBLE
			sprintf(disassembly, "BIT 7, H");
#endif
			break;
		case 0x7D:
			// BIT 7, (HL)
			tmp_c = memory_read8(cpu_registers.HL);
			tmp_c = (GET_BIT(tmp_c, 7) << Z_FLAG);
			cpu_registers.FLAG = tmp_c & FLAG_PRECOMPUTE_BIT;
			cycles = 16;
#ifdef DISASSEMBLE
			sprintf(disassembly, "BIT 7, (HL)");
#endif
			break;
		case 0x7F:
			// BIT 7, A
			tmp_c = (GET_BIT(cpu_registers.A, 7) << Z_FLAG);
			cpu_registers.FLAG = tmp_c & FLAG_PRECOMPUTE_BIT;
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
	cpu_registers.FLAG = (!(((cpu_registers.A & 0xF) - (val)) & 0xF)) <<H_FLAG;
	cpu_registers.FLAG |= 1 << N_FLAG;
	
	/**
	Remember:
		tmp_c is unsigned
		If tmp_c is signed, 129 wraps and makes neg.
	*/
	val = cpu_registers.A - val;
	if(val > 128)
		cpu_registers.FLAG |= (1 << C_FLAG);
	else if(!val)
		cpu_registers.FLAG |= (1 << Z_FLAG);
}
