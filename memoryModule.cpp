#include "memoryModule.hpp"

bool MemoryModule::loadOS(std::string path)
{
	std::ifstream in(path, std::ios::binary);
	if (in.good() == false)
	{
		in.close();
		in.open("OS/" + path, std::ios::binary);
		if (in.good() == false)
		{
			in.close();
			in.open("OS/" + path + ".os", std::ios::binary);
			if (in.good() == false)
			{
				in.close();
				return false;
			}
		}
	}
	std::string input((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	for (int i = 0; i < input.length(); i++)
	{
		if (i == 0x4000)
			break;
		MemoryModule::romPages[0]->data[i] = input[i];
	}
	in.close();
	return true;
}

MemoryModule::MemoryModule()
{
	MemoryModule::maxPage = 0x40;
	MemoryModule::maxROM = 0x1;
	MemoryModule::lockPageZero = false;
	for (uint32_t i = 0; i < MemoryModule::maxPage; i++)
	{
		MemoryModule::pages.insert(std::pair<uint32_t, MemoryModule::MemoryPage*>(i, new MemoryModule::MemoryPage));
		MemoryModule::pages[i]->readOnly = false;
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
		throw std::runtime_error("Memory module control access denied");
		return;
	}
	addr -= memoryModulePort;
	value = value >= MemoryModule::maxPage ? MemoryModule::maxPage - 1 : value;
	if (addr == 1)
	{
		MemoryModule::addressSpace[1] = MemoryModule::pages[value];
		return;
	}
	if (addr == 2)
	{
		MemoryModule::addressSpace[2] = MemoryModule::pages[value];
		return;
	}
	if (addr == 3)
	{
		MemoryModule::addressSpace[3] = MemoryModule::pages[value];
		return;
	}
	if (addr == 0)
	{
		if (MemoryModule::lockPageZero)
		{
			throw std::runtime_error("Attempted to change page 0 when switching prohibited");
			return;
		}
		if ((value & 0b100000) == 0b100000)
			MemoryModule::addressSpace[0] = MemoryModule::romPages[0];
		else
			MemoryModule::addressSpace[0] = MemoryModule::pages[value];
	}
}