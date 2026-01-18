#include "chip8.h"
#include <cassert>
#include <cstring>
#include <filesystem>
#include <fstream>

Chip8::Chip8()
    :
    memory(new uint8_t[MEMORY_SIZE]),
    video(new uint32_t[VIDEO_WIDTH * VIDEO_HEIGHT]),
	pc(START_ADDRESS), randGen(std::random_device{}()), randByte(0, 255U)
{
    memset(memory.get(), 0, sizeof(uint8_t) * MEMORY_SIZE);
    memset(video.get(), 0, sizeof(uint32_t) * VIDEO_WIDTH * VIDEO_HEIGHT);

	for (unsigned int i = 0; i < FONTSET_SIZE; ++i)
	{
        memory.get()[FONTSET_START_ADDRESS + i] = fontset[i];
	}


	table[0x0] = &Chip8::Table0;
	table[0x1] = &Chip8::OP_1nnn;
	table[0x2] = &Chip8::OP_2nnn;
	table[0x3] = &Chip8::OP_3xkk;
	table[0x4] = &Chip8::OP_4xkk;
	table[0x5] = &Chip8::OP_5xy0;
	table[0x6] = &Chip8::OP_6xkk;
	table[0x7] = &Chip8::OP_7xkk;
	table[0x8] = &Chip8::Table8;
	table[0x9] = &Chip8::OP_9xy0;
	table[0xA] = &Chip8::OP_Annn;
	table[0xB] = &Chip8::OP_Bnnn;
	table[0xC] = &Chip8::OP_Cxkk;
	table[0xD] = &Chip8::OP_Dxyn;
	table[0xE] = &Chip8::TableE;
	table[0xF] = &Chip8::TableF;

	for (size_t i = 0; i <= 0xE; i++)
	{
		table0[i] = &Chip8::OP_NULL;
		table8[i] = &Chip8::OP_NULL;
		tableE[i] = &Chip8::OP_NULL;
	}

	table0[0x0] = &Chip8::OP_00E0;
	table0[0xE] = &Chip8::OP_00EE;

	table8[0x0] = &Chip8::OP_8xy0;
	table8[0x1] = &Chip8::OP_8xy1;
	table8[0x2] = &Chip8::OP_8xy2;
	table8[0x3] = &Chip8::OP_8xy3;
	table8[0x4] = &Chip8::OP_8xy4;
	table8[0x5] = &Chip8::OP_8xy5;
	table8[0x6] = &Chip8::OP_8xy6;
	table8[0x7] = &Chip8::OP_8xy7;
	table8[0xE] = &Chip8::OP_8xyE;

	tableE[0x1] = &Chip8::OP_ExA1;
	tableE[0xE] = &Chip8::OP_Ex9E;

	for (size_t i = 0; i <= 0x65; i++)
	{
		tableF[i] = &Chip8::OP_NULL;
	}

	tableF[0x07] = &Chip8::OP_Fx07;
	tableF[0x0A] = &Chip8::OP_Fx0A;
	tableF[0x15] = &Chip8::OP_Fx15;
	tableF[0x18] = &Chip8::OP_Fx18;
	tableF[0x1E] = &Chip8::OP_Fx1E;
	tableF[0x29] = &Chip8::OP_Fx29;
	tableF[0x33] = &Chip8::OP_Fx33;
	tableF[0x55] = &Chip8::OP_Fx55;
	tableF[0x65] = &Chip8::OP_Fx65;


}

Chip8::~Chip8()
{
    video.reset();
    memory.reset();
}

void Chip8::LoadROM(const char* fileName)
{
	std::ifstream file(fileName, std::ios::binary);

	if (file.is_open() && file.good())
	{
		auto size = std::filesystem::file_size({ fileName });
        std::unique_ptr<char> buffer(new char[size]);

        file.read(buffer.get(), size);
		file.close();

		for (uint i = 0; i < size; ++i)
		{
            memory.get()[START_ADDRESS + i] = buffer.get()[i];
		}
	}

}

void Chip8::OP_00E0()
{
    memset(video.get(), 0, sizeof(uint32_t) * VIDEO_WIDTH * VIDEO_HEIGHT);
}

void Chip8::OP_00EE()
{
	--sp;
	pc = stack[sp];
}


void Chip8::OP_1nnn()
{
	uint16_t address = (opcode & 0x0FFFu);
	pc = address;
}

void Chip8::OP_2nnn()
{
	uint16_t address = (opcode & 0x0FFFu);
	stack[sp] = pc;
	++sp;
	pc = address;
}

void Chip8::OP_3xkk()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t kk = (opcode & 0x00FFu);

	if (registers[Vx] == kk)
	{
		pc += 2;
	}

}

void Chip8::OP_4xkk()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t kk = (opcode & 0x00FFu);

	if (registers[Vx] != kk)
	{
		pc += 2;
	}

}

void Chip8::OP_5xy0()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] == registers[Vy])
	{
		pc += 2;
	}
}

void Chip8::OP_6xkk()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = (opcode & 0x00FFu);

	registers[Vx] = byte;
}

void Chip8::OP_7xkk()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = (opcode & 0x00FFu);

	registers[Vx] += byte;
}

void Chip8::OP_8xy0()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] = registers[Vy];
}

void Chip8::OP_8xy1()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] |= registers[Vy];
}


void Chip8::OP_8xy2()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] &= registers[Vy];
}


void Chip8::OP_8xy3()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] ^= registers[Vy];
}


void Chip8::OP_8xy4()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	uint16_t sum = registers[Vx] + registers[Vy];

	if (sum > 255u)
	{
		registers[0xf] = 1;
	}
	else
	{
		registers[0xf] = 0;
	}

	registers[Vx] = sum & 0xffu;
}


void Chip8::OP_8xy5()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] > registers[Vy])
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}


	registers[Vx] -= registers[Vy];
}


void Chip8::OP_8xy6()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[0xf] = registers[Vy] & 1u;

	registers[Vx] = registers[Vy] >> 1u;
}

void Chip8::OP_8xy7()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vy] > registers[Vx])
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}

	registers[Vx] = registers[Vy] - registers[Vx];
}

void Chip8::OP_8xyE()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[0xf] = (registers[Vy] & (0x80)) >> 7u;  // 0x80 = 1 << 7

	registers[Vx] = registers[Vy] << 1u;
}

void Chip8::OP_9xy0()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] != registers[Vy])
	{
		pc += 2;
	}
}

void Chip8::OP_Annn()
{
	uint16_t address = (opcode & 0x0FFFu);
	index_register = address;
}

void Chip8::OP_Bnnn()
{

	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint16_t address = (opcode & 0x0FFFu);

	pc = address + registers[Vx];
}

void Chip8::OP_Cxkk()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	registers[Vx] = randByte(randGen) & byte;
}

void Chip8::OP_Dxyn()
{

	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
	uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;


	uint8_t sprite_height = (opcode & 0x000Fu);

	registers[0xF] = 0;

	for (uint row = 0; row < sprite_height; row++)
	{
        uint8_t spriteByte = memory.get()[index_register + row];

		for (uint col = 0; col < 8; col++)
		{
			uint8_t sprite_pixel = (spriteByte) & (0x80u >> col);

            uint32_t* screenPixel = &video.get()[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

			if (sprite_pixel)
			{
				if (*screenPixel == 0xFFFFFFFF)
				{
					registers[0xF] = 1;
				}
				*screenPixel ^= 0xFFFFFFFF;
			}

		}
	}
}

void Chip8::OP_Ex9E()
{

	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	uint8_t key = registers[Vx];

	if (keypad[key])
	{
		pc += 2;
	}

}

void Chip8::OP_ExA1(){

	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	uint8_t key = registers[Vx];

	if (!keypad[key])
	{
		pc += 2;
	}

}
void Chip8::OP_Fx07()
{

	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	registers[Vx] = delay_timer;
}

void Chip8::OP_Fx0A()
{

	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	if (keypad[0])
	{
		registers[Vx] = 0;
	}
	else if (keypad[1])
	{
		registers[Vx] = 1;
	}
	else if (keypad[2])
	{
		registers[Vx] = 2;
	}
	else if (keypad[3])
	{
		registers[Vx] = 3;
	}
	else if (keypad[4])
	{
		registers[Vx] = 4;
	}
	else if (keypad[5])
	{
		registers[Vx] = 5;
	}
	else if (keypad[6])
	{
		registers[Vx] = 6;
	}
	else if (keypad[7])
	{
		registers[Vx] = 7;
	}
	else if (keypad[8])
	{
		registers[Vx] = 8;
	}
	else if (keypad[9])
	{
		registers[Vx] = 9;
	}
	else if (keypad[10])
	{
		registers[Vx] = 10;
	}
	else if (keypad[11])
	{
		registers[Vx] = 11;
	}
	else if (keypad[12])
	{
		registers[Vx] = 12;
	}
	else if (keypad[13])
	{
		registers[Vx] = 13;
	}
	else if (keypad[14])
	{
		registers[Vx] = 14;
	}
	else if (keypad[15])
	{
		registers[Vx] = 15;
	}
	else
	{
		pc -= 2;
	}
}

void Chip8::OP_Fx15(){

	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	delay_timer = registers[Vx];
}
void Chip8::OP_Fx18(){

	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	sound_timer = registers[Vx];
}
void Chip8::OP_Fx1E(){

	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	index_register += registers[Vx];
}
void Chip8::OP_Fx29(){

	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t digit = registers[Vx];

	index_register = FONTSET_START_ADDRESS + (5 * digit);

}
void Chip8::OP_Fx33(){
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t value = registers[Vx];

    memory.get()[index_register + 2] = value % 10;
	value /= 10;

    memory.get()[index_register + 1] = value % 10;
	value /= 10;

    memory.get()[index_register] = value % 10;


}
void Chip8::OP_Fx55(){
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i)
	{
        memory.get()[index_register + i] = registers[i];
	}


}
void Chip8::OP_Fx65(){

	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i)
	{
        registers[i] = memory.get()[index_register + i];
	}

}

uint32_t* Chip8::getVideo()
{
    return video.get();
}

uint8_t* Chip8::getKeypad()
{
	return keypad;
}

void Chip8::Cycle()
{
    opcode = (memory.get()[pc] << 8u) | memory.get()[pc + 1];

	pc += 2;

	((*this).*(table[(opcode & 0xF000u) >> 12u]))();

	if (delay_timer > 0)
	{
		--delay_timer;
	}

	if (sound_timer > 0)
	{
		--sound_timer;
	}
}
