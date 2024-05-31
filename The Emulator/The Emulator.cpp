#include <iostream>
#include "gpuModule.hpp"
#include "ioModule.hpp"
#include "memoryModule.hpp"
#include "ioBridge.hpp"
#include <thread>
#include <cstdlib>
#include <windows.h>


//External source start
//https://helloacm.com/modern-getch-implementation-on-windows-cc/
TCHAR getch() {
	DWORD mode, cc;
	HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
	if (h == NULL) {
		return 0;
	}
	GetConsoleMode(h, &mode);
	SetConsoleMode(h, mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));
	TCHAR c = 0;
	ReadConsole(h, &c, 1, &cc, NULL);
	SetConsoleMode(h, mode);
	return c;
}
//External source end

int main()
{
	GpuModule gpu;
	std::thread th = std::thread(&GpuModule::display, &gpu);

	MemoryModule mem;

	IoBridge bridge;

	Z80 cpu(memoryRead, memoryWrite, ioRead, ioWrite, &bridge);

	IoModule io(&cpu);

	bridge.init(&gpu, &io, &mem);

	//Set color white
	gpu.controlInput(gpuModulePort, 0b11100000);
	gpu.controlInput(gpuModulePort, 0b00111111);

	//Set pointer to 10x10
	gpu.controlInput(gpuModulePort, 0b11000000);
	gpu.controlInput(gpuModulePort, 10);
	gpu.controlInput(gpuModulePort, 0);
	gpu.controlInput(gpuModulePort, 10);
	gpu.controlInput(gpuModulePort, 0);

	//Draw character
	gpu.controlInput(gpuModulePort, 0b01110000);
	gpu.controlInput(gpuModulePort, 'Z');

	//Draw character
	gpu.controlInput(gpuModulePort, 0b01110000);
	gpu.controlInput(gpuModulePort, '8');

	//Draw character
	gpu.controlInput(gpuModulePort, 0b01110000);
	gpu.controlInput(gpuModulePort, '0');

	//Draw character
	gpu.controlInput(gpuModulePort, 0b01110000);
	gpu.controlInput(gpuModulePort, ' ');

	//Draw character
	gpu.controlInput(gpuModulePort, 0b01110000);
	gpu.controlInput(gpuModulePort, 'P');

	//Draw character
	gpu.controlInput(gpuModulePort, 0b01110000);
	gpu.controlInput(gpuModulePort, 'C');

	//Set pointer to 10x30
	gpu.controlInput(gpuModulePort, 0b11000000);
	gpu.controlInput(gpuModulePort, 10);
	gpu.controlInput(gpuModulePort, 0);
	gpu.controlInput(gpuModulePort, 30);
	gpu.controlInput(gpuModulePort, 0);

	//Draw character
	gpu.controlInput(gpuModulePort, 0b01110000);
	gpu.controlInput(gpuModulePort, '>');

	std::string inStr = "";
	do
	{
		system("cls");
		std::cout << "Z80 PC Emulator\n===============\nEnter path to the OS file\n\n>";
		std::cin >> inStr;
	} while (mem.loadOS(inStr) == false);

	int in = 0;
	while (in != 4)
	{
		system("cls");
		std::cout << "Z80 PC Emulator\n===============\n1. Keyboard emulation\n2. GPU module programming\n3. I/O module programming\n4. Exit\n\n>";
		std::cin >> in;
		if (in == 1)
		{
			system("cls");
			std::cout << "Keyboard emulation\n==================\nWrite ~ to go back\n\n>";
			TCHAR k;
			while ((k = getch()) != '~')
			{
				io.keyboardInput(k);
			}
		}
		else if (in == 2)
		{
			while (in != -1)
			{
				system("cls");
				std::cout << "GPU programming\n===============\n-1 to go back\n\n>";
				std::cin >> in;
				if (in >= 0)
					gpu.controlInput(gpuModulePort, in);
			}
		}
		else if (in == 3)
		{
			while (in != -1)
			{
				system("cls");
				if (in == -2)
				{
					std::string str = " ";
					str[0] = io.busOutput(ioModulePort);
					std::cout << "I/O programming\n===============\n-1 to go back, -2 to get single output\n\nI/O response: " << std::to_string(str[0]) << " '" << str << "'\n\n>";
				}
				else
				{
					std::cout << "I/O programming\n===============\n-1 to go back, -2 to get single output\n\n>";
				}
				std::cin >> in;
				if (in >= 0)
					io.busInput(ioModulePort, in);
			}
		}
	}

	gpu.turnOff();
	th.join();
}