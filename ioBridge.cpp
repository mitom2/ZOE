#include "ioBridge.hpp"

unsigned char ioRead(void* arg, unsigned short port)
{
	if (port == ioModulePort || port == ioModulePort + 1)
	{
		return ((IoBridge*)arg)->ioObj->busOutput(port);
	}
	return 0;
}

void ioWrite(void* arg, unsigned short port, unsigned char value)
{
	if (port == memoryModulePort || port == memoryModulePort + 1 || port == memoryModulePort + 2 || port == memoryModulePort + 3)
	{
		((IoBridge*)arg)->memObj->controlOperation(port, value);
	}
	else if (port == gpuModulePort || port == gpuModulePort + 1)
	{
		((IoBridge*)arg)->gpuObj->controlInput(port, value);
	}
	else if (port == ioModulePort || port == ioModulePort + 1)
	{
		((IoBridge*)arg)->ioObj->busInput(port, value);
	}
}

void IoBridge::init(GpuModule* gpu, IoModule* io, MemoryModule* mem)
{
	IoBridge::gpuObj = gpu;
	IoBridge::ioObj = io;
	IoBridge::memObj = mem;
}


unsigned char memoryRead(void* arg, unsigned short addr)
{
	for (int i = 0; i < 4; i++)
	{
		if (addr < 0x4000)
		{
			return ((IoBridge*)arg)->memObj->addressSpace[i]->data[addr];
		}
		addr -= 0x4000;
	}
	throw std::runtime_error("Memory read beyond address space");
	return 0x76;
}

void memoryWrite(void* arg, unsigned short addr, unsigned char value)
{
	for (int i = 0; i < 4; i++)
	{
		if (addr < 0x4000)
		{
			if (((IoBridge*)arg)->memObj->addressSpace[i]->readOnly == false)
				((IoBridge*)arg)->memObj->addressSpace[i]->data[addr] = value;
			return;
		}
		addr -= 0x4000;
	}
	throw std::runtime_error("Memory write beyond address space");
}