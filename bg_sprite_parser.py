'''
Prints out a sprite given an address
Created by Matthew Levy
6/17/19
'''

data = open('mem_dump.dat', 'rb').read()

def printTile(tile):
	for j in range(8):
		for i in range(8):
			'''
			The method for this is dumb.
			Take 2 bytes: b1 and b2
			
			b1:        0 1 1 1 0 1 0 1
			b2:        0 0 0 0 1 0 1 1
			bit color: 0 1 1 1 2 1 2 3
			    (Used to determine palete color)
			'''
			bits = ((tile[j*2] >> (7 - i)) & 0x1)
			bits |= (((tile[j*2+1] >> (7 - i)) & 0x1) << 0x1)
			
			if bits: print('x', end='')
			else: print(' ', end='')
		print('')

printTile(data[0x8010:0x8010 + 16])
print('\n-----\n')
printTile(data[0x8020:0x8020 + 16])
print('\n-----\n')
printTile(data[0x8030:0x8030 + 16])
print('\n-----\n')
printTile(data[0x8190:0x8190 + 16])
print('\n-----\n')
