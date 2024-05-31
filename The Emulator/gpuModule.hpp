#pragma once
#include <SFML/Graphics.hpp>
#include <list>
#include <mutex>
#include <cmath>
#include <fstream>

#define hideCur true

#define gpuModulePort 0b10101010
#define gpuSpeed 2000
#define characterSizeX 20
#define characterSizeY 32

#define cmd_ClearScreen				0b10000000
#define cmd_SetPointer				0b11000000
#define cmd_MovePointerLeftUp		0b11000010
#define cmd_MovePointerRightDown	0b11000011
#define cmd_setColor				0b11100000
#define cmd_drawPixel				0b01000000
#define cmd_drawLine				0b01100000
#define cmd_drawChar				0b01110000
#define cmd_drawRectangle			0b01111000

class GpuModule
{
	std::mutex listMtx;
	std::list<unsigned char> commands;

	bool createCharacterFile(char character, int x, int y);

	bool characterPixel(char character, int x, int y);

	void interpretCommand(sf::Image& pixels, float& cx, float& cy, uint32_t& color, float& dx, float& dy, float& step, float& ctr);

public:

	void display();

	void controlInput(unsigned short address, unsigned char value);

};