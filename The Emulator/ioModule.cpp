#include "ioModule.hpp"

void IoModule::readStorage(uint32_t sectorNum)
{
    std::ifstream in("ioDisk/" + std::to_string(sectorNum) + ".io");
    if (in.good() == false)
    {
        in.close();
        for (int i = 0; i < 512; i++)
        {
            IoModule::storageData.push_back(0);
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
    if (IoModule::intEnabled == true)
    {
        IoModule::cpu->generateIRQ(0);
    }
}

void IoModule::writeStorage(uint32_t sectorNum, uint32_t address, unsigned char value)
{
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
        throw std::exception("Could not create disk entry");
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

IoModule::IoModule(Z80* cpuObj)
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
}

void IoModule::busInput(unsigned short port, unsigned char value)
{
    if (port != ioModulePort && port != ioModulePort + 1)
    {
        throw std::exception("IO module bus input access denied");
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
                IoModule::cpu->generateIRQ(0);
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
        else if (value == cmd_selectHID)
        {
            IoModule::current = 3;
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
            throw std::exception("Storage device return mode 0 is not currently supported");
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
            throw std::exception("Storage device return mode 0 is not currently supported");
        }
        else if (value == cmd_selSectStorage)
        {
            IoModule::current = 11;
        }
        else if (value == cmd_beginRdStorage)
        {
            IoModule::readStorage(IoModule::selSector);
            if (IoModule::intEnabled == true)
                IoModule::cpu->generateIRQ(0);
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
        return;
    }

    if (IoModule::current==3)
    {
        IoModule::current = 0;
        IoModule::ptrHID = (value & 0b00001111);
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
            throw std::exception("Selected unsupported storage write source");
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
        IoModule::writeStorage(IoModule::selSector, IoModule::storageWriteAddress, value);
        IoModule::storageWriteAddress++;
    }
}

unsigned char IoModule::busOutput(unsigned short port)
{
    if (port != ioModulePort && port != ioModulePort + 1)
    {
        throw std::exception("IO module bus output access denied");
        return 0;
    }

    if (IoModule::current == 1)
    {
        if (IoModule::rdProg == 0)
        {
            IoModule::rdProg++;
            return 0b00100010;
        }
        if (IoModule::rdProg == 1)
        {
            IoModule::rdProg++;
            return (keyboardID << 4) + (IoModule::keyboardData.size() > 0 ? 0b0100 : 0b0000);
        }
        if (IoModule::rdProg == 2)
        {
            IoModule::rdProg=0;
            IoModule::current = 0;
            return storageID << 4;
        }
    }
    if (IoModule::current == 2)
    {
        IoModule::current = 0;
        return 0b00000000;
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
            unsigned char buff = IoModule::storageData.front();
            IoModule::storageData.pop_front();
            if (IoModule::storageData.size() == 0)
            {
                IoModule::current = 0;
            }
            return buff;
    }
    throw std::exception("IO bus output reached end without data acquisition");
    return 0;
}

void IoModule::keyboardInput(unsigned char value)
{
    IoModule::keyboardData.push_back(value);
    if (IoModule::intEnabled == true)
    {
        IoModule::cpu->generateIRQ(0);
    }
}
