#pragma once
#include "memoryModule.hpp"
#include "gpuModule.hpp"

GpuModule* gpuObj;

unsigned char ioRead(void* arg, unsigned short port);

void ioWrite(void* arg, unsigned short port, unsigned char value);