#include <iostream>
#include "gpuModule.hpp"
#include <thread>

int main()
{
	GpuModule gpu;
	std::thread th = std::thread(&GpuModule::display, &gpu);

	//Set color blue
	gpu.controlInput(gpuModulePort, 0b11100000);
	gpu.controlInput(gpuModulePort, 0b00000011);

	//Set pointer to 64x228
	gpu.controlInput(gpuModulePort, 0b11000000);
	gpu.controlInput(gpuModulePort, 64);
	gpu.controlInput(gpuModulePort, 0);
	gpu.controlInput(gpuModulePort, 228);
	gpu.controlInput(gpuModulePort, 0);

	//Draw rectangle to 255x255
	gpu.controlInput(gpuModulePort, 0b01111000);
	gpu.controlInput(gpuModulePort, 255);
	gpu.controlInput(gpuModulePort, 0);
	gpu.controlInput(gpuModulePort, 255);
	gpu.controlInput(gpuModulePort, 0);

	//Set color red
	gpu.controlInput(gpuModulePort, 0b11100000);
	gpu.controlInput(gpuModulePort, 0b00110000);

	//Set pointer to 100x200
	gpu.controlInput(gpuModulePort, 0b11000000);
	gpu.controlInput(gpuModulePort, 100);
	gpu.controlInput(gpuModulePort, 0);
	gpu.controlInput(gpuModulePort, 200);
	gpu.controlInput(gpuModulePort, 0);

	//Draw pixel
	gpu.controlInput(gpuModulePort, 0b01000000);

	//Set color green
	gpu.controlInput(gpuModulePort, 0b11100000);
	gpu.controlInput(gpuModulePort, 0b00001100);

	//Set pointer to 128x64
	gpu.controlInput(gpuModulePort, 0b11000000);
	gpu.controlInput(gpuModulePort, 128);
	gpu.controlInput(gpuModulePort, 0);
	gpu.controlInput(gpuModulePort, 64);
	gpu.controlInput(gpuModulePort, 0);

	//Draw line to 630x470
	gpu.controlInput(gpuModulePort, 0b01100000);
	gpu.controlInput(gpuModulePort, 0b01110110);
	gpu.controlInput(gpuModulePort, 0b10);
	gpu.controlInput(gpuModulePort, 0b11010110);
	gpu.controlInput(gpuModulePort, 0b1);

	//Set color white
	gpu.controlInput(gpuModulePort, 0b11100000);
	gpu.controlInput(gpuModulePort, 0b00111111);

	//Set pointer to 228x10
	gpu.controlInput(gpuModulePort, 0b11000000);
	gpu.controlInput(gpuModulePort, 228);
	gpu.controlInput(gpuModulePort, 0);
	gpu.controlInput(gpuModulePort, 10);
	gpu.controlInput(gpuModulePort, 0);

	//Draw character
	gpu.controlInput(gpuModulePort, 0b01110000);
	gpu.controlInput(gpuModulePort, '+');

	//Move pointer right by 5
	gpu.controlInput(gpuModulePort, 0b11000011);
	gpu.controlInput(gpuModulePort, 5);
	gpu.controlInput(gpuModulePort, 0);
	gpu.controlInput(gpuModulePort, 0);
	gpu.controlInput(gpuModulePort, 0);

	//Draw character
	gpu.controlInput(gpuModulePort, 0b01110000);
	gpu.controlInput(gpuModulePort, '1');

	//Draw character
	gpu.controlInput(gpuModulePort, 0b01110000);
	gpu.controlInput(gpuModulePort, 'A');

	//Move pointer right-down by 15x35
	gpu.controlInput(gpuModulePort, 0b11000011);
	gpu.controlInput(gpuModulePort, 5);
	gpu.controlInput(gpuModulePort, 0);
	gpu.controlInput(gpuModulePort, 35);
	gpu.controlInput(gpuModulePort, 0);

	//Draw line to 25x5
	gpu.controlInput(gpuModulePort, 0b01100000);
	gpu.controlInput(gpuModulePort, 5);
	gpu.controlInput(gpuModulePort, 0);
	gpu.controlInput(gpuModulePort, 25);
	gpu.controlInput(gpuModulePort, 0);

	//Move pointer left-up by 15x3
	gpu.controlInput(gpuModulePort, 0b11000011);
	gpu.controlInput(gpuModulePort, 15);
	gpu.controlInput(gpuModulePort, 0);
	gpu.controlInput(gpuModulePort, 3);
	gpu.controlInput(gpuModulePort, 0);

	//Set color purple
	gpu.controlInput(gpuModulePort, 0b11100000);
	gpu.controlInput(gpuModulePort, 0b00110011);

	//Draw pixel
	gpu.controlInput(gpuModulePort, 0b01000000);

	//Set color white
	gpu.controlInput(gpuModulePort, 0b11100000);
	gpu.controlInput(gpuModulePort, 0b00111111);

	//Set pointer to 5x150
	gpu.controlInput(gpuModulePort, 0b11000000);
	gpu.controlInput(gpuModulePort, 5);
	gpu.controlInput(gpuModulePort, 0);
	gpu.controlInput(gpuModulePort, 150);
	gpu.controlInput(gpuModulePort, 0);

	//Display all characters
	for (unsigned char c = 0; c < 255; c++)
	{
		gpu.controlInput(gpuModulePort, 0b01110000);
		gpu.controlInput(gpuModulePort, c);
	}

	int in = 0;
	while (in != -1)
	{
		std::cin >> in;
 		gpu.controlInput(gpuModulePort, in);
	}

	th.join();
}