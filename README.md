# GameBoy Emulator

# Completed
- Implement cycle tracker to allow "simultaneous" execution of instructions, graphics, audio, etc
- Dump sprites to text file
	- PURPOSE: Confirm loading successfully
- Confirm full processing of boot image
- Improve cycle tracking loop
- Improve memory manager
	- Segment restricting (Prevent access to restricted regions)
	- Easier checking for interrupts
	- I/O addresses
- Determine addresses for drawing pixels
- Created basic debugger
- UI for drawing of LCD screen
- Implement interrupts
- SBC
	- A = A - n - cy
	- Treat like: A = A - n
	- Need check for when adding cy
		- Something == 0 is what I think it will be
- For SUB
	- How can tmp_s < 0x00 (tmp_s is unsigned)

# TODO
- Implement all opcode
	- Abstract out common code
	- Improve ADD and SUB functions
		- Confirm they work as intended

- Implement sprites
- Implement DMA transfer
	- 0xFFB8 is address for it
	- Need this before implementing sprites
- Implement joypad control
- Implement ROM banking

