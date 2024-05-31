#pragma once
#include <unordered_map>
#include <cstdint>

#define memoryModulePort 0b10111100

class MemoryModule
{

public:

	struct MemoryPage
	{
		unsigned char data[0x4000];
		bool readOnly;
	};

	uint32_t maxPage; //Number of pages in memory to create, default 0x40
	uint32_t maxROM; //Number of ROM pages in memory to create (ROM will be the lowest pages), default 0x1
	MemoryModule::MemoryPage* addressSpace[4];
	std::unordered_map<uint32_t, MemoryModule::MemoryPage*> pages;
	std::unordered_map<uint32_t, MemoryModule::MemoryPage*> romPages;
	bool lockPageZero; //Define whether lower 16kB of memory should be locked to page 0, default true

	/*
	POSSIBLE BUG IN HARDWARE:
	Do not change lower 16kB in memory from ROM page 0 to another page, this may cause undefined behaviour!
	*/

	MemoryModule();

	void controlOperation(unsigned short addr, unsigned char value);

};

unsigned char memoryRead(void* arg, unsigned short addr);

void memoryWrite(void* arg, unsigned short addr, unsigned char value);