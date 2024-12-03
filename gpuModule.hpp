#pragma once
#include <SFML/Graphics.hpp>
#include <list>
#include <mutex>
#include <cmath>
#include <fstream>
#include <stdexcept>

#define hideCur true
#define gpuCharGenThr 96

#define gpuModulePort 0b10101010
#define gpuSpeed 2000
#define characterSizeX 11
#define characterSizeY 16

#define cmd_ClearScreen				0b10000000
#define cmd_SetPointer				0b11000000
#define cmd_MovePointerLeftUp		0b11000010
#define cmd_MovePointerRightDown	0b11000011
#define cmd_setColor				0b11100000
#define cmd_drawPixel				0b01000000
#define cmd_drawLine				0b01100000
#define cmd_drawChar				0b01110000
#define cmd_drawRectangle			0b01111000

/// <summary>
/// This class emulates the GPU module.
/// </summary>
class GpuModule
{
	/// <summary>
	/// Mutex for the commands list.
	/// </summary>
	std::mutex listMtx;

	/// <summary>
	/// Commands received by the GPU module.
	/// </summary>
	std::list<unsigned char> commands;

	/// <summary>
	/// Display will only be visible when enabled=true.
	/// </summary>
	bool enabled;

	/// <summary>
	/// Generates file for character, which is used to display it on the emulated screen.
	/// </summary>
	/// <param name="character">Character</param>
	/// <param name="x">Width</param>
	/// <param name="y">Height</param>
	/// <returns>True if no issues were encountered, false otherwise</returns>
	bool createCharacterFile(char character, int x, int y);

	/// <summary>
	/// Checks if pixel with coordinates (x,y) belongs to the character (should be colored).
	/// </summary>
	/// <param name="character">Character</param>
	/// <param name="x">x</param>
	/// <param name="y">y</param>
	/// <returns>True if it belongs, false otherwise</returns>
	bool characterPixel(char character, int x, int y);

	/// <summary>
	/// Emulates command from the front of the commands list.
	/// </summary>
	/// <param name="pixels">Screen pixels</param>
	/// <param name="cx">cx</param>
	/// <param name="cy">cy</param>
	/// <param name="color">Selected color</param>
	/// <param name="dx">dx</param>
	/// <param name="dy">dy</param>
	/// <param name="step">Step</param>
	/// <param name="ctr">Command counter</param>
	void interpretCommand(sf::Image& pixels, float& cx, float& cy, uint32_t& color, float& dx, float& dy, float& step, float& ctr);

public:

	/// <summary>
	/// Main display loop.
	/// </summary>
	void display();

	/// <summary>
	/// Input detected.
	/// </summary>
	/// <param name="address">Address bus contents.</param>
	/// <param name="value">Data bus contents.</param>
	void controlInput(unsigned short address, unsigned char value);

	/// <summary>
	/// Disables display.
	/// </summary>
	void turnOff();

};