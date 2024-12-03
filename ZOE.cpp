#include <iostream>
#include "gpuModule.hpp"
#include "ioModule.hpp"
#include "memoryModule.hpp"
#include "ioBridge.hpp"
#include <thread>
#include <cstdlib>
#include <fstream>

#ifdef _WIN32
	#include <windows.h>
#else
	#include "curses.h"
#endif

int getchar()
{
	#ifdef _WIN32
		//External source start
		//https://helloacm.com/modern-getch-implementation-on-windows-cc/
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
		//External source end
	#else
		initscr();
		int ch=getch();
		endwin();
		return ch;
	#endif
}


unsigned long long maxDebugLogLength = 0;

/// <summary>
/// CPU time.
/// </summary>
uint32_t cpuTime;

/// <summary>
/// Begins CPU execution.
/// </summary>
/// <param name="cpu">CPU pointer</param>
/// <param name="ioMod">I/O module pointer</param>
void executeCPU(Z80* cpu, IoModule* ioMod)
{
	cpu->setConsumeClockCallback([&](void* arg, int clock) -> void
		{
			cpuTime = clock;
		});
	std::ofstream dLog("debug.log");
	if (dLog.good() == false)
	{
		std::cout << "Failed to create log file. Stopping power-up sequence.\n";
		return;
	}
	unsigned long long dCtr = 0;
		dLog.flush();
	cpu->setDebugMessage([&](void* arg, const char* message) -> void {
		dCtr++;
		dLog.flush();
		if (dCtr <= maxDebugLogLength)
		{
			time_t t1 = time(NULL);
			struct tm* t2 = localtime(&t1);
			dLog << "[" << t2->tm_hour << ":" << t2->tm_min << ":" << t2->tm_sec << "]: " << message << "\n";
			dLog.flush();
			if (dCtr == maxDebugLogLength)
				dLog << "Log max size reached.";
		}
		});
		/*cpu->addBreakOperand(0xFE, 61, [&](void* arg, unsigned char* opcode, int opcodeLength) -> void {
		std::cout << "Breakpoint reached.";
		});*/
	cpu->execute();
	dLog.close();
}

/// <summary>
/// Main function, entry point for the app.
/// </summary>
int main(int argc, char** argv)
{
	try
	{
		cpuTime = 0;

		GpuModule gpu;
		std::thread th = std::thread(&GpuModule::display, &gpu);

		MemoryModule mem;

		IoBridge bridge;

		Z80 cpu(memoryRead, memoryWrite, ioRead, ioWrite, &bridge);

		IoModule io(&cpu, &cpuTime);

		bridge.init(&gpu, &io, &mem);

		std::string inStr = "";
		do
		{
			system("cls");
			std::cout << "Z80 PC Emulator\n===============\nEnter path to the OS file or enter OS name\n\n>";
			std::cin >> inStr;
		} while (mem.loadOS(inStr) == false);

		inStr = "";
		do
		{
			system("cls");
			std::cout << "Z80 PC Emulator\n===============\nEnter max log size:\n\n>";
			std::cin >> inStr;
		} while (!(maxDebugLogLength = std::strtoull(inStr.c_str(), NULL, 0)));

		std::thread cpuTh(&executeCPU, &cpu, &io);

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
				int k;
				while ((k = getchar()) != '~')
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

		cpu.requestBreak();
		cpuTh.join();
		gpu.turnOff();
		th.join();
	}
	catch(std::runtime_error& e)
	{
		std::cout << "EXCEPTION: " << e.what() << "\n";
	}
}