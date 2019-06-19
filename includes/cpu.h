#ifndef __CPU_H
#define __CPU_H

#include <stdio.h>

/**
Bit manipulation
*/
#define GET_BIT(src, bit) ((src >> bit) & 0x1)
#define CLEAR_BIT(src, bit) (src & ~(0x1 << bit))
#define SET_BIT(src, bit) (src | (0x1 << bit))

/**
Bit for each flag
----------------
7 6 5 4 3 2 1 0
Z N H C 0 0 0 0
*/
#define Z_FLAG 7
#define N_FLAG 6
#define H_FLAG 5
#define C_FLAG 4

/**
Precomputed Flag is AND'ed with cpu_registers.FLAG
This allows for modification of only the result based flag,
leaving the static changes to be done compile-time
*/
// Z - Dynamic, N - Reset, H - Reset, C - Reset
#define FLAG_PRECOMPUTE_XOR (1 << Z_FLAG)

// Z - Dynamic, N - Reset, H - Set, C - Unchanged
#define FLAG_PRECOMPUTE_BIT ((1 << Z_FLAG) | (1 << H_FLAG) | (1 << C_FLAG))

/**
For Flags not able to be precomputed
*/
// Z - Dynamic, N - Reset, H - Set if carry from bit 3, C - Unchanged
#define FLAG_COMPUTE_INC(value) (\
	((!value) << Z_FLAG) |\
	((!(((value & 0xF) + 1) & 0xF)) << H_FLAG) |\
	(1 << C_FLAG)\
)

// Z - Dynamic, N - Set, H - Set if no borrow from bit 4, C - Unchanged
#define FLAG_COMPUTE_DEC(value) (\
	((!value) << Z_FLAG) |\
	(1 << N_FLAG) |\
	((!(((value & 0xF) - 1) & 0xF)) << H_FLAG) |\
	(1 << C_FLAG)\
)

/**
15..8 7..0
  A    F
  B    C
  D    E
  H    L
    SP
    PC

Zero Flag (Z):
	This bit is set when the result of a math operation
	is zero or two values match when using the CP
	instruction.
Subtract Flag (N):
	This bit is set if a subtraction was performed in the
	last math instruction.
Half Carry Flag (H):
	This bit is set if a carry occurred from the lower
	nibble in the last math operation.
Carry Flag (C):
	This bit is set if a carry occurred from the last
	math operation or if register A is the smaller value
	when executing the CP instruction.
*/
struct registers {
	union {
		struct {
			unsigned char F;
			unsigned char A;
		};
		unsigned short AF;
	};
	union {
		struct {
			unsigned char C;
			unsigned char B;
		};
		unsigned short BC;
	};
	union {
		struct {
			unsigned char E;
			unsigned char D;
		};
		unsigned short DE;
	};
	union {
		struct {
			unsigned char L;
			unsigned char H;
		};
		unsigned short HL;
	};
	
	unsigned short PC;
	unsigned short SP;
	
	unsigned char FLAG;
};

struct cpu_state {
	unsigned char running;
	unsigned long long total_cycles;
	struct registers registers;
};

void cpu_init();
void cpu_reset();

struct cpu_state cpu_getState();
void cpu_setState(struct cpu_state state);
void cpu_printState();

unsigned char cpu_step();

extern struct cpu_state cpu_state;

#ifdef DISASSEMBLE
extern char disassembly[256];
extern short disassembly_pc;
#endif

#endif
