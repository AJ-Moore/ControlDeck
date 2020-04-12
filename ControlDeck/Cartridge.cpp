// Copyright © Allan Moore April 2020

#include "Cartridge.h"

namespace ControlDeck
{
	bool Cartridge::Load(std::string path)
	{
        m_path = path;
        std::string nesFile = m_path;
        std::ifstream file(nesFile, std::ios::binary | std::ios::ate);
        std::streamsize size = file.tellg();

        if (!file.good())
        {
            printf("Failed to open file");
            return false;
        }

        file.seekg(0, std::ios::beg);
        std::vector<char> buffer(size);
        if (file.read(buffer.data(), size))
        {
            // NES
            printf("Loading: %s\n", nesFile.c_str());
            printf("================================\n");
            printf("%c", buffer[0]);
            printf("%c", buffer[1]);
            printf("%c", buffer[2]);
            printf("%c\n", buffer[3]);

            // every rom has at least 1 16k 16384 byte rom bank
            printf("16k PRG Rom Banks: %i\n", buffer[4]);
            printf("8k CHR VRom Banks: %i\n", buffer[5]);
            printf("8k Ram Banks: %i\n", buffer[8]);
            printf("Region pal/ntfc: %s\n", buffer[9] == 0 ? "PAL" : "NTFC");

            m_prgRomBanks = buffer[4];
            m_chrVRamBanks = buffer[5];
            m_flags = buffer[6];
            
            // Read rom banks, starting from bit 16
            m_romBanks.clear();
            uint32 bankOffset = 16;
            for (int i = 0; i < m_prgRomBanks; ++i)
            {
                printf("Loading 16k rom bank [%i]\n", i);
                std::vector<ubyte> bank = std::vector<ubyte>(buffer.begin() + bankOffset, 
                                                            buffer.begin() + (bankOffset + m_romBankSize));
                bankOffset += m_romBankSize;
                m_romBanks.push_back(bank);
            }

            if (m_prgRomBanks == 1)
            {
                m_romBanks.push_back(std::vector<ubyte>(m_romBanks[0]));
            }

        }

        file.close();
        m_bDoneLoad = true;

        return  true;
	}
    const std::vector<ubyte>& Cartridge::GetBank(uint16 bankNumber)
    {
        if (bankNumber > m_romBanks.size())
        {
            throw("Unable to load bank from cartridge");
            return std::vector<ubyte>();
        }

        m_romBanks[bankNumber];
    }
}
