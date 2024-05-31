#pragma once
#include "memoryModule.hpp"
#include "gpuModule.hpp"
#include "ioModule.hpp"

class IoBridge
{
public:

	GpuModule* gpuObj;

	IoModule* ioObj;

	MemoryModule* memObj;

	void init(GpuModule* gpu, IoModule* io, MemoryModule* mem);

};


unsigned char ioRead(void* arg, unsigned short port);

void ioWrite(void* arg, unsigned short port, unsigned char value);

unsigned char memoryRead(void* arg, unsigned short addr);

void memoryWrite(void* arg, unsigned short addr, unsigned char value);