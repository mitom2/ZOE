#pragma once
#include <chrono>
#include <fstream>
#include <list>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <semaphore>
#include "z80-suzukiplan/z80.hpp"

#define ioModulePort 0b10001110

/*
Command groups for IO:

0b00xxxxxx - Controller interface
0b01xxxxxx - HID interface
0b10xxxxxx - Mass storage interface
0b11xxxxxx - Other devices interface

*/

/*
Device table info:

First read: 0b12345678
123 - number of connected HID devices
456 - number of connected other devices
7 - number of mass storage devices
8 - error flag

Next [123] reads: 0bABCDEFGH
ABCD - device id
E - ready for data flag
F - data available flag
G - device error flag
H - controller error flag

Next [456 x 2] reads: 0bABCDEFGH, 0bIJKLMNOP
ABCD - device id
EF - type (0b00 - no communication, 0b01 - write only, 0b10 - read only, 0b11 - read/write possible)
G - ready for data flag
H - data available flag
IJKL - custom device id
M - custom device flag 1
N - custom device flag 2
O - device error flag
P - controller error flag

Next read (if mass storage device is present): 0bABCDEFGH
ABCD - device id
EF - reserved
G - device error flag
H - controller error flag
*/

/*
Errors table:

First read: 8-bit number of errors

Next [num. of errors] reads: 0bABCDEFGH
ABCD - device ID (0 for controller)
EFGH - error code

Error codes (controller):
0b0000 - HID device selection failed
0b0001 - other device selection failed
0b0010 - requested output when controller not in read mode

Error codes (other - timer):

0b0000 - attempted data read
*/

#define cmd_begin			0b00000001 //Initialize possible connections. Int is generated after this task is finished, unless disabled. After read, return devices table.
#define cmd_getDevices		0b00100000 //After next read, return devices table.
#define cmd_cancelRead		0b00101111 //Cancel reading from the controller.
#define cmd_disableInt		0b00000010 //Turn off interrupts. Read will be possible only using select, check and read combination.
#define cmd_enableInt		0b00000100 //Enable interrupts.
#define cmd_getErrors		0b00001000 //Returns errors table.
#define cmd_clearErrors		0b00010000 //Clears all internal errors.
#define cmd_selectHID		0b01110000 //Select HID device for communication. 4 lowest bits are device ID.
#define cmd_checkDataHID	0b01100000 //Check if data is available on selected HID device. 0b00000000 if it is, 0b00000001 otherwise.
#define cmd_readHID			0b01000001 //Prepare data from selected HID device to be read.
#define cmd_writeHID		0b01000010 //Prepare the selected HID device to receive data in next writes.
#define cmd_rdyForDataHID	0b01000100 //Checks whether the HID device is ready to receive data. 0b00000000 if it is, 0b00000001 otherwise.
#define cmd_numStorage		0b10010000 //Checks whether storage device with id specified by 4 lowest bits is connected. 0b00000000 if it is, 0b00000001 otherwise.
#define cmd_availStorage	0b10000000 //Returns number of storage devices connected. Keep in mind that only 1 storage device at a time is supported, any more will cause an error.
#define cmd_setRetMdStorage	0b10001000 //Sets return mode for storage device. LSB=0 (default) - results in [optional INT], BUSRQ, Write to RAM. LSB=1 - results in [optional INT] and return of table on D bus.
#define cmd_checkStorage	0b10110000 //Checks whether data from storage device is available. 0b00000000 if it is, 0b00000001 otherwise.
#define cmd_readStorage		0b10100000 //Returns data table. Depending on return mode, all 512 results are either available through multiple reads or they are sent to specified RAM location.
#define cmd_ramDestStorage	0b10111000 //Sets RAM address to which data should be saved when using return mode 0. Expects two more writes with the actual address (lower part of address first).
#define cmd_selSectStorage	0b10111100 //Selects sector in the storage device. Sector number is expected as four more inputs, from LSB to MSB.
#define cmd_beginRdStorage	0b10111110 //Begins accessing selected sector. If interruptions are enabled, one will be generated after completion.
#define cmd_writeStorage	0b10111111 //Writes to the selected sector. Expects more writes: 0b10000000 (bus source) or 0b10111111 (RAM source), then size of the data to be written (2 inputs, from LSB to MSB), then starting address of write in sector (2 inputs, LSB to MSB). If BUS source was selected, specified number of data inputs are expected.
#define cmd_chkWrPrgStorage	0b10111101 //Checks whether the controller is currently writing to the storage device. 0b00000000 if it is, 0b00000001 otherwise.
#define cmd_rdyFDtaStorage	0b10111011 //Checks whether the storage device is ready to receive data. 0b00000000 if it is, 0b00000001 otherwise.
#define cmd_selectOther		0b11110000 //Selects other device specified by the lowest 4 bits.
#define cmd_getTypeOther	0b11100000 //Returns type of the other connected device (0b00001100 - read/write, 0b00001000 - read only, 0b00000100 - write only, 0b00000000 - no communication)
#define cmd_rdyForDataOther	0b11010000 //Checks whether the other device is ready to receive data. 0b00000000 if it is, 0b00000001 otherwise.
#define cmd_checkDataOther	0b11000000 //Check if data is available on selected other device. 0b00000000 if it is, 0b00000001 otherwise.
#define cmd_readOther		0b11000001 //Read data from other device. Behaviour is determined by the device itself.
#define cmd_writeOther		0b11000010 //Write data to the other device. Behaviour is determined by the device itself.
#define cmd_setFlagsOther	0b11000100 //Set flag(s) of other device. Can set multiple flags at once. Flags to be changed are specified by lowest bits. Expects one more input with new flag values (LSB=flag1, MSB=flag2).
#define cmd_getFlagsOther	0b11001000 //Returns flags: 0bM000000L - M=flag 2, L=flag 1.
#define cmd_getCstmIdOther	0b11001100 //Returns custom ID selected other device.
#define cmd_setCstmIdOther	0b11001111 //Sets new custom ID for the selected other device. Expects one more input with new ID on lower 4 bits.

//WARNING: currently supports only storage ret mode 1 and write mode 0b10000000 (but default ret mode is still 0)!
//WARNING: cmd_chkWrPrgStorage and cmd_rdyFDtaStorage might not be working!

#define keyboardID 0b00001111
#define keyboardVector 8

#define storageID 0b00001000
#define storageSimulatedReadDelay 16//In uS
#define storageSimulatedWriteDelay 16//In uS
#define storageVector 4

/*
Timer is write-only device with two custom flags:
	-Flag 0bX1 enables the timer.
	-Flag 0b1X informs the timer that it should reset to the beginning instead of stopping when reaching the threshold.

Writing into the device sets internal threshold that will trigger INT when reached. INT will be triggered even if it is disabled in the controller.
*/
#define timerID	 0b00000101
#define timerDeviceType 0b00000100
#define timerSpeedModifier 6
#define timerVector 0

#define beginVector 12

class IoModule
{

	Z80* cpu;

	bool intEnabled;

	std::list<unsigned char> errors;

	std::list<unsigned char> keyboardData;

	std::list<unsigned char> storageData;

	unsigned char ptrHID;

	unsigned char ptrStorage;

	unsigned char ptrOther;

	unsigned char customTimerId;

	unsigned char timerFlags;

	uint32_t selSector;

	uint32_t storageWriteSize;

	uint32_t storageWriteAddress;

	std::list<unsigned char> timerErrors;

	std::mutex storageMtx;

	std::binary_semaphore intInProg;

	/*
	0 - Command Rd
	1 - Device table Rd
	2 - Errors table Rd
	3 - Selected other device type Rd
	4 - Other device 1st B of data (LSB) Wr
	5 - HID data Rd
	6 - HID data Wr
	7 - HID ready for data Rd
	8 - 0b00000000 Rd
	9 - 0b00000001 Rd
	10 - Storage data Rd
	11 - 1st B of sector number Wr
	12 - 2nd B of sector number Wr
	13 - 3rd B of sector number Wr
	14 - 4th B of sector number Wr
	15 - 0b10000000 Wr
	16 - 1st B of storage write data size Wr
	17 - 2nd B of storage write data size Wr
	18 - Other flag 1 Wr
	19 - Other flag 2 Wr
	20 - 1st B of storage write data address in sector Wr
	21 - 2nd B of storage write data address in sector Wr
	22 - Storage write data input Wr
	23 - Other flags 1 and 2 Wr
	24 - Other flags Rd
	25 - Other device custom ID Rd
	26 - Other device custom ID Wr
	27 - Other device 2nd B of data Wr
	28 - Other device 3rd B of data Wr
	29 - Other device 4th B of data (MSB) Wr
	*/
	short current;

	short rdProg;

	short wrProg;

	uint32_t* cpuCycles;

	uint32_t timerVal;

	uint32_t timerTh;

	std::thread* timerThread;

	bool stopTimer;

	void readStorage(uint32_t sectorNum, int delay);

	void writeStorage(uint32_t sectorNum, uint32_t address, unsigned char value, int delay);

	void timerSimulator();

public:

	IoModule(Z80* cpuObj, uint32_t* cpuTime);

	~IoModule();

	void busInput(unsigned short port, unsigned char value);

	unsigned char busOutput(unsigned short port);

	void keyboardInput(unsigned char value);

	void intFinished();

};