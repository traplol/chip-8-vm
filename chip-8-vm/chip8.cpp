/*
	Copyright(c) 2014 Max Mickey

	This software is provided 'as-is', without any express or implied
	warranty.In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions :

	1. The origin of this software must not be misrepresented; you must not
	claim that you wrote the original software.If you use this software
	in a product, an acknowledgment in the product documentation would be
	appreciated but is not required.

	2. Altered source versions must be plainly marked as such, and must not be
	misrepresented as being the original software.

	3. This notice may not be removed or altered from any source
	distribution. 
*/

#include "chip8.h"
u8 chip8_fontset[80] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, //0
	0x20, 0x60, 0x20, 0x20, 0x70, //1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
	0x90, 0x90, 0xF0, 0x10, 0x10, //4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
	0xF0, 0x10, 0x20, 0x40, 0x40, //7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
	0xF0, 0x90, 0xF0, 0x90, 0x90, //A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
	0xF0, 0x80, 0x80, 0x80, 0xF0, //C
	0xE0, 0x90, 0x90, 0x90, 0xE0, //D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
	0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

#ifdef LOGGING
void Chip8::dumpstate()
{

	FILE* log_file = fopen(LOG_FILE, "a");
	fprintf(log_file, "========REGISTERS=========\n");
	fprintf(log_file, "PC: %04X\n", PC);
	fprintf(log_file, "SP: %04X\n", SP);
	fprintf(log_file, "I : %04X\n", I);
	for (int i = 0; i < V_SIZE; ++i)
	{
		fprintf(log_file, "V%X: %02X\n", i, V[i]);
	}
	fprintf(log_file, "=======INSTRUCTION========\n");
	fprintf(log_file, "INSTRUCTION: %04X\n", instruction);
	fprintf(log_file, "     OPCODE: %X\n", op);
	fprintf(log_file, "          X: %X\n", x);
	fprintf(log_file, "          Y: %X\n", y);
	fprintf(log_file, "        NNN: %03X\n", n3);
	fprintf(log_file, "         NN: %02X\n", n2);
	fprintf(log_file, "          N: %X\n", n1);
	fprintf(log_file, "===========MISC===========\n");
	fprintf(log_file, "delay_timer: %X\n", delay_timer);
	fprintf(log_file, "sound_timer: %X\n", sound_timer);
	fprintf(log_file, "  key_press: %X\n", key_press);
	fprintf(log_file, "==========================\n");
	fclose(log_file);
}
#endif

void Chip8::init()
{
#ifdef LOGGING
	FILE* log_file = fopen(LOG_FILE, "wb+");
	printf("Creating log file %s\n", LOG_FILE);
	fclose(log_file);
#endif
	std::fill(MEMORY, MEMORY + MEM_SIZE, 0);
	std::fill(STACK, STACK + STACK_SIZE, 0);
	std::fill(V, V + V_SIZE, 0);
	std::fill(GFX, GFX + GFX_SIZE, 0);
	std::fill(KEY_STATE, KEY_STATE + NUM_OF_KEYS, 0);

	PC = 0x200;
	SP = 0;
	I = 0;
	op = 0;

	delay_timer = 0;
	sound_timer = 0;

	for (int i = 0; i < 80; ++i)
	{
		MEMORY[i] = chip8_fontset[i];
	}
}

bool Chip8::load_rom(char* filename)
{
	bool success = true;
	FILE* file = fopen(filename, "rb");
	if (file == NULL)
	{
		printf("Can't find file: %s\n", filename);
		success = false;
	}
	if (success)
	{
		fseek(file, 0, SEEK_END);
		long size = ftell(file);
		rewind(file);
		char* buf = (char*)malloc(sizeof(char)* size);

		size_t result = fread(buf, 1, size, file);
		if (result != size)
		{
			printf("file read error\n");
			success = false;
		}
		if (success)
		{
			if ((4096 - 512) > size)
			{
				for (int i = 0; i < size; ++i)
				{
					MEMORY[i + 512] = buf[i];
				}
			}
			else
			{
				printf("ROM too big for memory.");
				success = false;
			}
		}
		fclose(file);
		free(buf);
	}
	return success;
}

void Chip8::run_cycle()
{
	decode();
#ifdef LOGGING
	dumpstate();
#endif
	execute();
	if (delay_timer > 0)
	{
		delay_timer--;
	}
	if (sound_timer > 0)
	{
		if (sound_timer > 0)
		{
			printf("beep!\n");
		}
		sound_timer--;
	}
}

void Chip8::decode()
{
	u16 mem = (MEMORY[PC] << 8) + MEMORY[PC + 1];
	instruction = mem;
	op = GetOP(mem);
	x = GetX(mem);
	y = GetY(mem);
	n3 = GetN3(mem);
	n2 = GetN2(mem);
	n1 = GetN1(mem);
	PC += 2;
}

void Chip8::execute()
{
#ifdef DEBUGGER
	printf("Instruction: %X\n", instruction);
	printf("PC :%04X\n", PC);
	printf("Registers:\n");
	for (int i = 0; i < V_SIZE; ++i)
	{
		if (i > 0 && i % 4 == 0)
		{
			printf("\n");
		}
		printf("v%X:%2X  ", i, V[i]);
	}
	printf("\n-----------------------------\n");
#endif
	switch (op)
	{
	case 0x0:
		if (n2 == 0xE0)
		{
			// Clear the screen.
#ifdef DEBUGGER
			printf("cls\n");
#endif
			std::fill(GFX, GFX + GFX_SIZE, 0);
			draw_flag = true;
		}
		else if (n2 == 0xEE)
		{
			// Return from subroutine.
#ifdef DEBUGGER
			printf("rts\n");
#endif
			PC = STACK[SP--];
		}
		else
		{
			//Calls RCA 1802 program at address n3.
#ifdef DEBUGGER
			printf("RCA 1802 program at 0x%X\n", n3);
#endif
		}
		break;
		// Jumps to address n3.
	case 0x1:
#ifdef DEBUGGER
		printf("jmp %X\n", n3);
#endif
		PC = n3;
		break;
		// Calls subroutine at n3.
	case 0x2: 
#ifdef DEBUGGER
		printf("jsr %X\n", n3);
#endif
		STACK[++SP] = PC;
		PC = n3;
		break;
		// Skips the next instruction if V[x] == n2.
	case 0x3: 
#ifdef DEBUGGER
		printf("skeq v%X, %X\n", x, n2);
		printf("%X == %X\n", V[x], n2);
		printf("eval: %X\n", V[x] == n2);
#endif
		if (V[x] == n2)
			PC += 2;
		break;
		// Skips the next instruction if V[x] != n2.
	case 0x4: 
#ifdef DEBUGGER
		printf("skne v%X, %X\n", x, n2);
		printf("%X != %X\n", V[x], n2);
		printf("eval: %X\n", V[x] != n2);
#endif
		if (V[x] != n2)
			PC += 2;
		break;
		// Skips the next instruction if V[x] == V[y].
	case 0x5: 
#ifdef DEBUGGER
		printf("skeq v%X, v%X\n", x, y);
		printf("%X == %X\n", V[x], V[y]);
		printf("eval: %X\n", V[x] == V[y]);
#endif
		if (V[x] == V[y])
			PC += 2;
		break;
		// Sets V[x] to n2.
	case 0x6: 
#ifdef DEBUGGER
		printf("mov v%X, %X\n", x, n2);
#endif
		V[x] = n2;
		break;
		// Adds n2 to V[x].
	case 0x7: 
#ifdef DEBUGGER
		printf("add v%X, %X\n", x, n2);
		printf("eval: %X\n", V[x] + n2);
#endif
		V[x] = V[x] + n2;
		break;
	case 0x8:
		switch (n1)
		{
			// Sets V[x] to the value of V[y].
		case 0x0: 
#ifdef DEBUGGER
			printf("mov v%X, v%X\n", x, y);
#endif
			V[x] = V[y];
			break;
			// Sets V[x] to V[x] | V[y].
		case 0x1: 
#ifdef DEBUGGER
			printf("or v%X, v%X\n", x, y);
			printf("eval: %X\n", V[x] | V[y]);
#endif
			V[x] = V[x] | V[y];
			break;
			// Sets V[x] to V[x] & V[y].
		case 0x2: 
#ifdef DEBUGGER
			printf("and v%X, v%X\n", x, y);
			printf("eval: %X\n", V[x] & V[y]);
#endif
			V[x] = V[x] & V[y];
			break;
			// Sets V[x] to V[x] ^ V[y].
		case 0x3: 
#ifdef DEBUGGER
			printf("xor v%X, v%X\n", x, y);
			printf("eval: %X\n", V[x] ^ n2);
#endif
			V[x] = V[x] ^ V[y];
			break;
			// Adds V[y] to V[x]. V[0xF] is set to 1 when there's a carry
			// and 0 when there isn't.
		case 0x4: 
#ifdef DEBUGGER
			printf("add v%X, v%X\n", x, y);
			printf("eval: %X\n", V[x] + V[y]);
#endif
			V[x] = V[x] + V[y];
			V[0xF] = (V[x] < V[y]) ? 1 : 0;
#ifdef DEBUGGER
			printf("eval: vF: %X\n", V[0xF]);
#endif
			break;
			// V[y] is subtracted from V[x]. V[0xF] is set to 0 when there's
			// a borrow, and 1 when there isn't.
		case 0x5: 
#ifdef DEBUGGER
			printf("sub v%X, v%X\n", x, y);
			printf("eval: %X\n", V[x] - V[y]);
#endif
			V[0xF] = (V[x] > V[y]) ? 1 : 0;
			V[x] = V[x] - V[y];
#ifdef DEBUGGER
			printf("eval: vF: %X\n", V[0xF]);
#endif
			break;
			// Shifts V[x] right by 1. V[0xF] is set to LSB(V[x]) before
			// the shift.
		case 0x6:
#ifdef DEBUGGER
			printf("shr v%X\n", x);
			printf("eval: %X\n", V[x] >> 1);
			printf("eval: vF: %X\n", LSB(V[x]));
#endif
			V[0xF] = LSB(V[x]);
			V[x] = V[x] / 2;
			break;
			// Sets V[x] to V[y] minus V[x]. V[0xF] is set to 0 when there's
			// a borrow, and 1 when there isn't.
		case 0x7: 
#ifdef DEBUGGER
			printf("rsb v%X, v%X\n", x, y);
			printf("eval: %X\n", V[y] - V[x]);
#endif
			V[0xF] = (V[y] > V[x]) ? 1 : 0;
			V[x] = V[y] - V[x];
#ifdef DEBUGGER
			printf("eval: vF: %X\n", V[0xF]);
#endif
			break;
			// Shifts V[x] left by 1. V[0xF] is set to MSB(V[x]) before
			// the shift.
		case 0xE: 
#ifdef DEBUGGER
			printf("shl v%X\n", x);
			printf("eval: %X\n", V[x] << 1);
			printf("eval: vF: %X\n", MSB8(V[x]));
#endif
			V[0xF] = MSB8(V[x]);
			V[x] = V[x] * 2;
			break;
		}
		break;
		// Skips the next instruction if V[x] != V[y].
	case 0x9: 
#ifdef DEBUGGER
		printf("skne v%X, v%X\n", x, y);
		printf("eval: %X\n", V[x] != V[y]);
#endif
		if (V[x] != V[y])
			PC += 2;
		break;
		// Sets I to n3.
	case 0xA: 
#ifdef DEBUGGER
		printf("mvi %X\n", n3);
#endif
		I = n3;
		break;
		// Jumps to the address (n3 + V[0]).
	case 0xB: 
#ifdef DEBUGGER
		printf("jmi %X\n", n3);
		printf("eval: %X\n", n3 + V[0]);
#endif
		PC = (n3 + V[0]);
		break;
		// Sets V[x] to a random number & n2.
	case 0xC: 
#ifdef DEBUGGER
		printf("rand v%X, %X\n", x, n2);
#endif
		V[x] = (rand() % 0xFF) & n2;
#ifdef DEBUGGER
		printf("eval: %X\n", V[x]);
#endif
		break;
		// Draws a sprite at coordinate (V[x], V[y]) that has a width of 8 pixels
		// and a height of n1 pixels. Each row of 8 pixels is read as bit-coded
		// (with the MSB of each byte displayed on the left) starting from memory
		// location I; I value doesn't change after the execution of this
		// instruction. As described above, V[0xF] is set to 1 if any screen pixels
		// are flipped from set to unset when the sprite is drawn, and to 0 if that
		// doesn't happen.
	case 0xD:
#ifdef DEBUGGER
		printf("sprite v%X, v%X, %X\n", x, y, n1);
#endif
	{
		u16 x_ = V[x];
		u16 y_ = V[y];
		u16 height_ = n1;
		u16 pixel_ = 0;
		draw_flag = 1;
		for (int vert = 0; vert < height_; ++vert)
		{
			pixel_ = MEMORY[I + vert];
			for (int hori = 0; hori < 8; ++hori)
			{
				if ((pixel_ & (0x80 >> hori)) != 0)
				{
					if (GFX[x_ + hori + ((y_ + vert) * 64)] == 1)
					{
						V[0xF] = 1;
					}
					GFX[x_ + hori + ((y_ + vert) * 64)] ^= 1;
				}
			}
		}
	}
		break;
	case 0xE:
		if (n2 == 0x9E)
		{
			// Skips the next instruction if the key stored in V[x] is pressed.
#ifdef DEBUGGER
			printf("skpr %X\n", KEY_STATE[V[x]]);
			printf("eval: %X\n", KEY_STATE[V[x]] != 0);
#endif
			if (KEY_STATE[V[x]] != 0)
				PC += 2;
		}
		else if (n2 == 0xA1)
		{
			// Skips the next instruction if the key stored in V[x] isn't pressed.
#ifdef DEBUGGER
			printf("skup %X\n", KEY_STATE[V[x]]);
			printf("eval: %X\n", KEY_STATE[V[x]] == 0);
#endif
			if (KEY_STATE[V[x]] == 0)
				PC += 2;
		}
		break;
	case 0xF:
		switch (n2)
		{
			// Sets V[x] to the value of delay_timer.
		case 0x07:
#ifdef DEBUGGER
			printf("gdelay v%X\n", x);
			printf("eval: %X\n", delay_timer);
#endif
			V[x] = delay_timer;
			break;
			// A key press is awaited and then stored in V[x].
		case 0x0A:
#ifdef DEBUGGER
			printf("key v%X\n", x);
#endif
			for (int i = 0; i < NUM_OF_KEYS; ++i)
			{
				if (KEY_STATE[i] != 0)
				{
					V[x] = i;
					key_press = 1;
					break;
				}
			}
			if (!key_press)
			{
				PC -= 2;
			}
			break;
			// Sets delay_timer to V[x].
		case 0x15:
#ifdef DEBUGGER
			printf("sdelay v%X\n", x);
#endif
			delay_timer = V[x];
			break;
			// Sets sound_timer to V[x].
		case 0x18:
#ifdef DEBUGGER
			printf("ssound v%X\n", x);
#endif
			sound_timer = V[x];
			break;
			// Adds V[x] to I.
		case 0x1E:
#ifdef DEBUGGER
			printf("adi v%X\n", x);
			printf("eval: %X\n", I + V[x]);
#endif
			I = I + V[x];
			break;
			// Sets I to the location of the sprite for the character in V[x]. 
			// Characters 0-F are represented by a 4x5 font.
		case 0x29:
#ifdef DEBUGGER
			printf("adi v%X\n", x);
			printf("eval: %X\n", V[x] * 5);
#endif
			I = V[x] * 5;
			break;
			// Stores the binary-coded decimal representation of V[x], with
			// the most significant of 3 digits at the address in I, the middle digit
			// at I plus 1, and the least significant digit at I plus 2.
			// (In other words, take the decimal representation of V[x], place the
			//  hundreds digit in memory at location I, the tens at location I+1 and
			//  the ones digit at location I+2.)
		case 0x33:
#ifdef DEBUGGER
			printf("bcd v%X\n", x);
			printf("eval: %X\n", V[x] / 100);
			printf("eval: %X\n", (V[x] / 10) % 10);
			printf("eval: %X\n", (V[x] % 100) % 10);
#endif
			MEMORY[I] = V[x] / 100;
			MEMORY[I + 1] = (V[x] / 10) % 10;
			MEMORY[I + 2] = V[x] % 10;
			break;
			// Stores V[0] to V[x] in memory starting at address I.
		case 0x55:
#ifdef DEBUGGER
			printf("str v0-v%X\n", x);
#endif
			for (int i = 0; i <= x; ++i)
			{
				MEMORY[I + i] = V[i];
			}
			I += x + 1;
			break;
			// Fills V[0] to V[X] with values from memory starting at address I.
		case 0x65:
#ifdef DEBUGGER
			printf("ldr v0-v%X\n", x);
#endif
			for (int i = 0; i <= x; ++i)
			{
				V[i] = MEMORY[I + i];
			}
			I += x + 1;
			break;
		}
		break;
	}
#ifdef DEBUGGER
	printf("-----------------------------\n");
#endif
}