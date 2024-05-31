#include "memoryModule.hpp"

MemoryModule::MemoryModule()
{
	MemoryModule::maxPage = 0x40;
	MemoryModule::maxROM = 0x1;
	MemoryModule::lockPageZero = true;
	for (uint32_t i = 0; i < MemoryModule::maxPage; i++)
	{
		MemoryModule::pages.insert(std::pair<uint32_t, MemoryModule::MemoryPage*>(i, new MemoryModule::MemoryPage));
		memset(&(MemoryModule::pages[i]->data), 0, sizeof(MemoryModule::pages[i]->data));
	}
	for (uint32_t i = 0; i < MemoryModule::maxROM; i++)
	{
		MemoryModule::romPages.insert(std::pair<uint32_t, MemoryModule::MemoryPage*>(i, new MemoryModule::MemoryPage));
		MemoryModule::romPages[i]->readOnly = true;
		memset(&(MemoryModule::romPages[i]->data), 0, sizeof(MemoryModule::romPages[i]->data));
	}
	MemoryModule::addressSpace[0] = MemoryModule::romPages[0];
	MemoryModule::addressSpace[1] = MemoryModule::pages[0];
	MemoryModule::addressSpace[2] = MemoryModule::pages[0];
	MemoryModule::addressSpace[3] = MemoryModule::pages[0];
}

void MemoryModule::controlOperation(unsigned short addr, unsigned char value)
{
	if (addr != memoryModulePort && addr != memoryModulePort + 1 && addr != memoryModulePort + 2 && addr != memoryModulePort + 3)
	{
		throw std::exception("Memory module control access denied");
		return;
	}
	addr -= memoryModulePort;
	value = value >= MemoryModule::maxPage ? MemoryModule::maxPage - 1 : value;
	if (addr == 0)
	{
		MemoryModule::addressSpace[1] = MemoryModule::pages[value];
		return;
	}
	if (addr == 1)
	{
		MemoryModule::addressSpace[2] = MemoryModule::pages[value];
		return;
	}
	if (addr == 2)
	{
		MemoryModule::addressSpace[3] = MemoryModule::pages[value];
		return;
	}
	if (addr == 4)
	{
		if (MemoryModule::lockPageZero)
		{
			throw std::exception("Attempted to change page 0 when switching prohibited");
			return;
		}
		MemoryModule::addressSpace[0] = MemoryModule::romPages[value];
	}
}

unsigned char memoryRead(void* arg, unsigned short addr)
{
	for (int i = 0; i < 4; i++)
	{
		if (addr < 0x4000)
		{
			return ((MemoryModule*)arg)->addressSpace[i]->data[addr];
		}
		addr -= 0x4000;
	}
	throw std::exception("Memory read beyond address space");
	return 0x76;
}

void memoryWrite(void* arg, unsigned short addr, unsigned char value)
{
	for (int i = 0; i < 4; i++)
	{
		if (addr < 0x4000)
		{
			((MemoryModule*)arg)->addressSpace[i]->data[addr] = value;
			return;
		}
		addr -= 0x4000;
	}
	throw std::exception("Memory write beyond address space");
}