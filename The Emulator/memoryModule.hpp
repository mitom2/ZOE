#pragma once
#include <unordered_map>
#include <cstdint>
#include <string>
#include <fstream>

#define memoryModulePort 0b10111100

/// <summary>
/// This class emulates MEM module.
/// </summary>
class MemoryModule
{
public:

	/// <summary>
	/// Loads program to memory before emulation start.
	/// </summary>
	/// <param name="path">Path to the program</param>
	/// <returns>True is no issues appeared, false otherwise</returns>
	bool loadOS(std::string path);

	/// <summary>
	/// This struct emulates a single 16kB memory page.
	/// </summary>
	struct MemoryPage
	{
		/// <summary>
		/// Data stored.
		/// </summary>
		unsigned char data[0x4000];

		/// <summary>
		/// Is read only?
		/// </summary>
		bool readOnly;
	};

	/// <summary>
	/// Number of pages in memory to create, default 0x40
	/// </summary>
	uint32_t maxPage;

	/// <summary>
	/// Number of ROM pages in memory to create (ROM will be the lowest pages), default 0x1
	/// </summary>
	uint32_t maxROM;

	/// <summary>
	/// 64kB address space (4 pages)
	/// </summary>
	MemoryModule::MemoryPage* addressSpace[4];

	/// <summary>
	/// Map of all created RAM pages.
	/// </summary>
	std::unordered_map<uint32_t, MemoryModule::MemoryPage*> pages;

	/// <summary>
	/// Map of all created ROM pages.
	/// </summary>
	std::unordered_map<uint32_t, MemoryModule::MemoryPage*> romPages;

	/// <summary>
	/// Define whether lower 16kB of memory should be locked to page 0, default false.
	/// </summary>
	bool lockPageZero;

	/// <summary>
	/// Default constructor.
	/// </summary>
	MemoryModule();

	/// <summary>
	/// I/O operation detected.
	/// </summary>
	/// <param name="addr">Address bus contents.</param>
	/// <param name="value">Data bus contents.</param>
	void controlOperation(unsigned short addr, unsigned char value);

};