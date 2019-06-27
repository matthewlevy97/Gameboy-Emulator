# GameBoy Emulator

# TODO
- Implement all opcode
	- Abstract out common code
	- Improve ADD and SUB functions
		- Confirm they work as intended

- Implement sprites
- Implement DMA transfer
- Implement joypad control
- Implement ROM banking

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

