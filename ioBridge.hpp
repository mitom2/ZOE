#pragma once
#include "memoryModule.hpp"
#include "gpuModule.hpp"
#include "ioModule.hpp"

/// <summary>
/// This class serves as a bridge that directs outputs/inputs to proper modules.
/// </summary>
class IoBridge
{
public:

	/// <summary>
	/// GPU module.
	/// </summary>
	GpuModule* gpuObj;

	/// <summary>
	/// I/O module.
	/// </summary>
	IoModule* ioObj;

	/// <summary>
	/// MEM module.
	/// </summary>
	MemoryModule* memObj;

	/// <summary>
	/// Initializes class.
	/// </summary>
	/// <param name="gpu">GPU module</param>
	/// <param name="io">I/O module</param>
	/// <param name="mem">MEM module</param>
	void init(GpuModule* gpu, IoModule* io, MemoryModule* mem);

};

/// <summary>
/// Handles I/O module read operation.
/// </summary>
/// <param name="arg">arg</param>
/// <param name="port">Port number</param>
/// <returns>Data</returns>
unsigned char ioRead(void* arg, unsigned short port);

/// <summary>
/// Handles I/O module write operation.
/// </summary>
/// <param name="arg">arg</param>
/// <param name="port">Port number</param>
/// <param name="value">Data bus contents</param>
void ioWrite(void* arg, unsigned short port, unsigned char value);

/// <summary>
/// Handles MEM module read operation.
/// </summary>
/// <param name="arg">arg</param>
/// <param name="port">Port number</param>
/// <returns>Data</returns>
unsigned char memoryRead(void* arg, unsigned short addr);

/// <summary>
/// Handles MEM module write operation.
/// </summary>
/// <param name="arg">arg</param>
/// <param name="port">Port number</param>
/// <param name="value">Data bus contents</param>
void memoryWrite(void* arg, unsigned short addr, unsigned char value);