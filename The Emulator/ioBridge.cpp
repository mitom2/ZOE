#include "ioBridge.hpp"

unsigned char ioRead(void* arg, unsigned short port)
{
	return 0;
}

void ioWrite(void* arg, unsigned short port, unsigned char value)
{
	if (port == memoryModulePort || port == memoryModulePort + 1 || port == memoryModulePort + 2 || port == memoryModulePort + 3)
	{
		((MemoryModule*)arg)->controlOperation(port, value);
	}
	else if (port == gpuModulePort || port == gpuModulePort + 1)
	{
		gpuObj->controlInput(port, value);
	}
}
