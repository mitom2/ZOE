#include "ioModule.hpp"

void IoModule::readStorage(uint32_t sectorNum, int delay)
{
    std::this_thread::sleep_for(std::chrono::microseconds(delay));
    IoModule::storageMtx.lock();
    std::ifstream in("ioDisk/" + std::to_string(sectorNum) + ".io");
    if (in.good() == false)
    {
        in.close();
        for (int i = 0; i < 512; i++)
        {
            IoModule::storageData.push_back(0);
        }
        IoModule::storageMtx.unlock();
        if (IoModule::intEnabled == true)
        {
            IoModule::intInProg.acquire();
            std::this_thread::sleep_for(std::chrono::microseconds(2));
            IoModule::cpu->generateIRQ(storageVector);
        }
        return;
    }
    std::string str;
    std::getline(in, str);
    for (int i=0; i<str.length(); i++)
    {
        IoModule::storageData.push_back(str[i]);
    }
    in.close();
    IoModule::storageMtx.unlock();
    if (IoModule::intEnabled == true)
    {
        IoModule::intInProg.acquire();
        std::this_thread::sleep_for(std::chrono::microseconds(2));
        IoModule::cpu->generateIRQ(storageVector);
    }
}

void IoModule::writeStorage(uint32_t sectorNum, uint32_t address, unsigned char value, int delay)
{
    std::this_thread::sleep_for(std::chrono::microseconds(delay));
    std::vector<unsigned char> prev;
    std::ifstream in("ioDisk/" + std::to_string(sectorNum) + ".io");
    if (in.good() == false)
    {
        in.close();
        for (int i = 0; i < 512; i++)
        {
            prev.push_back(0);
        }
    }
    else
    {
        std::string str;
        std::getline(in, str);
        for (int i = 0; i < str.length(); i++)
        {
            prev.push_back(str[i]);
        }
        in.close();
    }
    for (int i = prev.size(); i < 512; i++)
    {
        prev.push_back(0);
    }
    std::ofstream out("ioDisk/" + std::to_string(sectorNum) + ".io");
    if (out.good() == false)
    {
        out.close();
        throw std::runtime_error("Could not create disk entry");
        return;
    }
    for (int i = 0; i < prev.size(); i++)
    {
        if (i == address)
            out << value;
        else
            out << prev[i];
    }
    out.close();
}

void IoModule::timerSimulator()
{
    uint32_t last = *(IoModule::cpuCycles);
    bool stopped = true;
    while (IoModule::stopTimer == false)
    {
        if (IoModule::timerFlags == 1 || IoModule::timerFlags == 3)
        {
            if (stopped == true)
            {
                stopped = false;
                uint32_t last = *(IoModule::cpuCycles);
            }
            IoModule::timerVal += ((*(IoModule::cpuCycles)) - last) * timerSpeedModifier;
            uint32_t last = *(IoModule::cpuCycles);
            if (IoModule::timerVal >= IoModule::timerTh)
            {
                IoModule::intInProg.acquire();
                std::this_thread::sleep_for(std::chrono::microseconds(2));
                IoModule::cpu->generateIRQ(timerVector);
                IoModule::timerVal = 0;
                if (IoModule::timerFlags == 1)
                {
                    IoModule::timerFlags = 0;
                    stopped = true;
                }
            }
        }
    }
}

IoModule::IoModule(Z80* cpuObj, uint32_t* cpuTime) : intInProg(1)
{
    IoModule::cpu = cpuObj;
    IoModule::intEnabled = true;
    IoModule::current = 0;
    IoModule::selSector = 0;
    IoModule::rdProg = 0;
    IoModule::storageWriteSize = 0;
    IoModule::storageWriteAddress = 0;
    IoModule::wrProg = 0;
    IoModule::ptrHID = 0;
    IoModule::ptrStorage = 0;
    IoModule::cpuCycles = cpuTime;
    IoModule::timerVal = 0;
    IoModule::ptrOther = 0;
    IoModule::customTimerId = 0;
    IoModule::timerTh = 0;
    IoModule::stopTimer = false;
    IoModule::timerFlags = 0;
    IoModule::intInProg.release();
    IoModule::timerThread = new std::thread(&IoModule::timerSimulator, this);
}

IoModule::~IoModule()
{
    IoModule::stopTimer = true;
    IoModule::timerThread->join();
    delete IoModule::timerThread;
}

void IoModule::busInput(unsigned short port, unsigned char value)
{
    if (port != ioModulePort && port != ioModulePort + 1)
    {
        throw std::runtime_error("IO module bus input access denied");
        return;
    }
    if (value == cmd_cancelRead)
    {
        IoModule::current = 0;
        return;
    }

    if (IoModule::current == 0)
    {
        if (value == cmd_begin)
        {
                IoModule::current = 1;
                if (IoModule::intEnabled == true)
                {
                    if (IoModule::intInProg.try_acquire() == true)
                    {
                        std::this_thread::sleep_for(std::chrono::microseconds(2));
                        IoModule::cpu->generateIRQ(beginVector);
                    }
                }
        }
        else if (value == cmd_getDevices)
        {
            IoModule::current = 1;
        }
        else if (value == cmd_disableInt)
        {
            IoModule::intEnabled = false;
        }
        else if (value == cmd_enableInt)
        {
            IoModule::intEnabled = true;
        }
        else if (value == cmd_getErrors)
        {
            IoModule::current = 2;
        }
        else if (value == cmd_clearErrors)
        {
            IoModule::errors.clear();
        }
        else if ((value & 0b11110000) == cmd_selectHID)
        {
            IoModule::current = 0;
            IoModule::ptrHID = (value & 0b00001111);
            if (IoModule::ptrHID != keyboardID)
                errors.push_back(0);
        }
        else if (value == cmd_checkDataHID)
        {
            if (IoModule::keyboardData.size() > 0)
                IoModule::current = 8;
            else
                IoModule::current = 9;
        }
        else if (value == cmd_readHID)
        {
            IoModule::current = 5;
        }
        else if (value == cmd_writeHID)
        {
            IoModule::current = 6;
        }
        else if (value == cmd_rdyForDataHID)
        {
            IoModule::current = 7;
        }
        else if ((value & 0b11110000) == cmd_numStorage)
        {
            if ((value & 0b00001111) == storageID)
                IoModule::current = 8;
            else
                IoModule::current = 9;
        }
        else if (value == cmd_availStorage)
        {
            IoModule::current = 9;
        }
        else if (value == cmd_setRetMdStorage)
        {
            throw std::runtime_error("Storage device return mode 0 is not currently supported");
        }
        else if (value == cmd_setRetMdStorage + 1)
        {
            IoModule::current = 0;
        }
        else if (value == cmd_checkStorage)
        {
            if (IoModule::storageData.size() > 0)
                IoModule::current = 8;
            else
                IoModule::current = 9;
        }
        else if (value == cmd_readStorage)
        {
            IoModule::current = 10;
        }
        else if (value == cmd_ramDestStorage)
        {
            throw std::runtime_error("Storage device return mode 0 is not currently supported");
        }
        else if (value == cmd_selSectStorage)
        {
            IoModule::current = 11;
        }
        else if (value == cmd_beginRdStorage)
        {
            std::thread t(&IoModule::readStorage, this, IoModule::selSector, storageSimulatedReadDelay);
            t.detach();
        }
        else if (value == cmd_writeStorage)
        {
            IoModule::current = 15;
        }
        else if (value == cmd_chkWrPrgStorage)
        {
            IoModule::current = 9;
        }
        else if (value == cmd_rdyFDtaStorage)
        {
            IoModule::current = 8;
        }
        else if ((value & 0b11110000) == cmd_selectOther)
        {
            IoModule::current = 0;
            IoModule::ptrOther = (value & 0b00001111);
            if (IoModule::ptrOther != timerID)
                errors.push_back(1);
        }
        else if (value == cmd_getTypeOther)
        {
            IoModule::current = 3;
        }
        else if (value == cmd_rdyForDataOther)
        {
            IoModule::current = 8;
        }
        else if (value == cmd_checkDataOther)
        {
            IoModule::current = 9;
        }
        else if (value == cmd_readOther)
        {
            IoModule::timerErrors.push_back(0);
        }
        else if (value == cmd_writeOther)
        {
            IoModule::current = 4;
        }
        else if ((value & 0b11111100) == cmd_setFlagsOther)
        {
            if(value == cmd_setFlagsOther + 1)
                IoModule::current = 18;
            else if (value == cmd_setFlagsOther + 2)
                IoModule::current = 19;
            else if (value == cmd_setFlagsOther + 3)
                IoModule::current = 23;
        }
        else if (value == cmd_getFlagsOther)
        {
            IoModule::current = 24;
        }
        else if (value == cmd_getCstmIdOther)
        {
            IoModule::current = 25;
        }
        else if (value == cmd_setCstmIdOther)
        {
            IoModule::current = 26;
        }
        return;
    }

    if (IoModule::current == 4)
    {
        if (IoModule::ptrOther == timerID)
        {
            IoModule::timerTh = 0;
            IoModule::timerTh += value;
            IoModule::current = 27;
        }
    }
    else if (IoModule::current == 6)
    {
        IoModule::current = 0;
    }
    else if (IoModule::current == 11)
    {
        IoModule::current = 12;
        IoModule::selSector = value;
    }
    else if (IoModule::current == 12)
    {
        IoModule::current = 13;
        IoModule::selSector += (value << 8);
    }
    else if (IoModule::current == 13)
    {
        IoModule::current = 14;
        IoModule::selSector += (value << 16);
    }
    else if (IoModule::current == 14)
    {
        IoModule::current = 0;
        IoModule::selSector += (value << 24);
    }
    else if (IoModule::current == 15)
    {
        IoModule::current = 16;
        if (value != 0b10000000)
            throw std::runtime_error("Selected unsupported storage write source");
    }
    else if (IoModule::current == 16)
    {
        IoModule::current = 17;
        IoModule::storageWriteSize = value;
    }
    else if (IoModule::current == 17)
    {
        IoModule::current = 20;
        IoModule::storageWriteSize += (value << 8);
    }
    else if (IoModule::current == 18)
    {
        IoModule::current = 0;
        if (IoModule::ptrOther == timerID)
            IoModule::timerFlags = (IoModule::timerFlags & 0b00000010) + value;
    }
    else if (IoModule::current == 19)
    {
        IoModule::current = 0;
        if (IoModule::ptrOther == timerID)
            IoModule::timerFlags = (IoModule::timerFlags & 0b00000001) + (value >> 6);
    }
    else if (IoModule::current == 20)
    {
        IoModule::current = 21;
        IoModule::storageWriteAddress = value;
    }
    else if (IoModule::current == 21)
    {
        IoModule::current = 22;
        IoModule::storageWriteAddress += (value << 8);
    }
    else if (IoModule::current == 22)
    {
        IoModule::wrProg++;
        if (wrProg >= IoModule::storageWriteSize)
        {
            IoModule::wrProg = 0;
            IoModule::current = 0;
        }
        IoModule::writeStorage(IoModule::selSector, IoModule::storageWriteAddress, value, storageSimulatedWriteDelay);
        IoModule::storageWriteAddress++;
    }
    else if (IoModule::current == 23)
    {
        IoModule::current = 0;
        if (IoModule::ptrOther == timerID)
            IoModule::timerFlags = (value >> 6) + (value & 0b00000001);
    }
    else if (IoModule::current == 26)
    {
        IoModule::current = 0;
        if (IoModule::ptrOther == timerID)
        {
            IoModule::customTimerId = value & 0b00001111;
        }
    }
    else if (IoModule::current == 27)
    {
        if (IoModule::ptrOther == timerID)
        {
            IoModule::timerTh += (value << 8);
            IoModule::current = 28;
        }
    }
    else if (IoModule::current == 28)
    {
        if (IoModule::ptrOther == timerID)
        {
            IoModule::timerTh += (value << 16);
            IoModule::current = 29;
        }
    }
    else if (IoModule::current == 29)
    {
        if (IoModule::ptrOther == timerID)
        {
            IoModule::timerTh += (value << 24);
            IoModule::current = 0;
        }
    }
}

unsigned char IoModule::busOutput(unsigned short port)
{
    if (port != ioModulePort && port != ioModulePort + 1)
    {
        throw std::runtime_error("IO module bus output access denied");
        return 0;
    }

    if (IoModule::current == 1)
    {
        if (IoModule::rdProg == 0)
        {
            IoModule::rdProg++;
            return 0b00100110 + (IoModule::errors.size() > 0 ? 1 : 0);
        }
        if (IoModule::rdProg == 1)
        {
            IoModule::rdProg++;
            if (IoModule::errors.size() == 0)
                return (keyboardID << 4) + (IoModule::keyboardData.size() > 0 ? 0b0100 : 0b0000);
            return (keyboardID << 4) + (IoModule::keyboardData.size() > 0 ? 0b0101 : 0b0001);
        }
        if (IoModule::rdProg == 2)
        {
            IoModule::rdProg++;
            return (timerID << 4) + 0b0110;
        }
        if (IoModule::rdProg == 3)
        {
            IoModule::rdProg++;
            return (IoModule::customTimerId << 4) + (IoModule::timerFlags << 2) + (IoModule::errors.size() == 0 ? 0 : 1);
        }
        if (IoModule::rdProg == 4)
        {
            IoModule::rdProg=0;
            IoModule::current = 0;
            return storageID << 4;
        }
    }
    if (IoModule::current == 2)
    {
        if (IoModule::rdProg == 0)
        {
            IoModule::rdProg = 1;
            return IoModule::errors.size() + IoModule::timerErrors.size();
        }
        if (IoModule::rdProg > 0)
        {
            if (IoModule::errors.size() > 0)
            {
                unsigned char res = IoModule::errors.front();
                IoModule::errors.pop_front();
                return res;
            }
            if (IoModule::timerErrors.size() > 0)
            {
                unsigned char res = IoModule::timerErrors.front() + (timerID << 4);
                IoModule::errors.pop_front();
                return res;
            }
            IoModule::rdProg = 0;
            IoModule::current = 0;
            return 0;
        }
    }
    if (IoModule::current == 3)
    {
        IoModule::current = 0;
        if (IoModule::ptrOther == timerID)
            return 0b00000100;
    }
    if (IoModule::current == 5)
    {
        IoModule::rdProg++;
        if (IoModule::rdProg == 1)
        {
            if (IoModule::keyboardData.size() == 0)
            {
                IoModule::rdProg = 0;
                IoModule::current = 0;
            }
            return IoModule::keyboardData.size();
        }
        else
        {
            unsigned char buff = IoModule::keyboardData.front();
            IoModule::keyboardData.pop_front();
            if (IoModule::keyboardData.size() == 0)
            {
                IoModule::rdProg = 0;
                IoModule::current = 0;
            }
            return buff;
        }
    }
    if (IoModule::current == 7)
    {
        IoModule::current = 0;
        return IoModule::keyboardData.size() > 0 ? 0b00000000 : 0b00000001;
    }
    if (IoModule::current == 8)
    {
        IoModule::current = 0;
        return 0b00000000;
    }
    if (IoModule::current == 9)
    {
        IoModule::current = 0;
        return 0b00000001;
    }
    if (IoModule::current == 10)
    {
        if (IoModule::storageData.size() > 0)
        {
            unsigned char buff = IoModule::storageData.front();
            IoModule::storageMtx.lock();
            IoModule::storageData.pop_front();
            IoModule::storageMtx.unlock();
            if (IoModule::storageData.size() == 0)
            {
                IoModule::current = 0;
            }
            return buff;
        }
        return 0;
    }
    if (IoModule::current == 24)
    {
        IoModule::current = 0;
        if (IoModule::ptrOther == timerID)
            return ((IoModule::timerFlags << 6) & 0b10000000) + (IoModule::timerFlags & 1);
    }
    if (IoModule::current == 25)
    {
        IoModule::current = 0;
        if (IoModule::ptrOther == timerID)
            return IoModule::customTimerId;
    }
    //throw std::runtime_error("IO bus output reached end without data acquisition");
    IoModule::errors.push_back(0b0010);
    return 0;
}

void IoModule::keyboardInput(unsigned char value)
{
    IoModule::keyboardData.push_back(value);
    if (IoModule::intEnabled == true)
    {
        IoModule::intInProg.acquire();
        IoModule::cpu->generateIRQ(keyboardVector);
    }
}

void IoModule::intFinished()
{
    IoModule::intInProg.release();
}
