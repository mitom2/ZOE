#include "memoryModule.hpp"

bool MemoryModule::loadOS(std::string path)
{
	std::ifstream in(path);
	if (in.good() == false)
	{
		in.close();
		return false;
	}
	unsigned char input;
	for (int i = 0; in >> input; i++)
	{
		if (i == 0x4000)
			break;
		MemoryModule::romPages[0]->data[i] = input;
	}
	in.close();
	return true;
}

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