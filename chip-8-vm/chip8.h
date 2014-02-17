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

#ifndef _CHIP8_H_
#define _CHIP8_H_

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <algorithm>
#include <ctime>

#define MEM_SIZE 4096
#define STACK_SIZE 16
#define GFX_WIDTH 64
#define GFX_HEIGHT 32
#define GFX_SIZE (GFX_WIDTH * GFX_HEIGHT)
#define NUM_OF_KEYS 16
#define V_SIZE 16

#ifdef _DEBUG
#define EMU_TICK_RATE 3
#else
#define EMU_TICK_RATE (1000.0 / 60)
#endif

#ifdef DEBUGGER
#define EMU_TICK_RATE -1
#endif

#ifdef LOGGING
#define LOG_FILE "dump.log"
#include <stdio.h>
#endif

#define GetOP(X) (X >> 12) 
#define GetX(X) ((X & 0xFFF) >> 8)
#define GetY(X) ((X & 0xFF) >> 4)
#define GetN3(X) (X & 0xFFF)
#define GetN2(X) (X & 0xFF)
#define GetN1(X) (X & 0xF)
#define LSB(X) (X & 1)
#define MSB16(X) (X >> 15)
#define MSB8(X) (X >> 7)

typedef unsigned char u8;
typedef unsigned short u16;

class Chip8
{
public:
	u8 GFX[GFX_SIZE];
	u8 KEY_STATE[NUM_OF_KEYS];
	u8 draw_flag;

	void init();
	bool load_rom(char* filename);
	void run_cycle();
private:
	u8 MEMORY[MEM_SIZE];
	u16 STACK[STACK_SIZE];

	u8 V[V_SIZE]; // Registers, 16 8-bit registers from V0 - VF.
	u16 I; // Address register, 16-bits wide.
	u16 PC; // Program counter.
	u16 SP; // Stack pointer.

	u8 delay_timer;
	u8 sound_timer;
	u8 key_press;

	u8 op, x, y, n2, n1;
	u16 n3, instruction;
	
	void decode();
	void execute();
#ifdef LOGGING
	void dumpstate();
#endif
};

#endif