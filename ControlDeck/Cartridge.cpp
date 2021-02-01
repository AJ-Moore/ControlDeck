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
            printf("Control Deck - A Nintendo Entertainment System Emulator by Allan\n");
            printf("Loading: %s\n", nesFile.c_str());
            printf("================================\n");
            printf("%c", buffer[0]);
            printf("%c", buffer[1]);
            printf("%c", buffer[2]);
            printf("%c\n", buffer[3]);

            // every rom has at least 1 16k 16384 byte rom bank
            printf("16k PRG Rom Banks: %i\n", buffer[4]);
            printf("8k CHR VRam Banks: %i\n", buffer[5]);
            printf("8k Ram Banks: %i\n", buffer[8]);
            printf("Region pal/ntfc: %s\n", buffer[9] == 0 ? "PAL" : "NTFC");

            printf("Mapper: %i\n", buffer[6] >> 4);

            m_prgRomBanks = buffer[4];
            m_chrVRamBanks = buffer[5];
            m_flags = buffer[6];
            
            // Read rom banks, starting from bit 16
            m_romBanks.clear();
            uint32 bankOffset = 16;
            for (int i = 0; i < m_prgRomBanks; ++i)
            {
                printf("Loading 16k rom bank [%i]\n", i);
                std::vector<uint8> bank = std::vector<uint8>(buffer.begin() + bankOffset, 
                                                            buffer.begin() + (bankOffset + m_romBankSize));
                bankOffset += m_romBankSize;
                m_romBanks.push_back(bank);
            }

            if (m_prgRomBanks == 1)
            {
                m_romBanks.push_back(std::vector<uint8>(m_romBanks[0]));
            }

            m_vramBanks.clear();

            // Load VRAM CHR banks
            for (int i = 0; i < m_chrVRamBanks; ++i)
            {
                printf("Loading 8k vram CHR Bank [%i]\n", i);
                std::vector<uint8> bank = std::vector<uint8>(buffer.begin() + bankOffset,
                                                                    buffer.begin() + (bankOffset + m_vramBankSize));
                bankOffset += m_vramBankSize;
                m_vramBanks.push_back(bank);
            }
        }

        file.close();
        m_bDoneLoad = true;

        return  true;
	}

    const std::vector<uint8>& Cartridge::GetPRGRomBank(uint16 bankNumber)
    {
        if (bankNumber > m_romBanks.size())
        {
            throw("Unable to load bank from cartridge");
            return std::vector<uint8>();
        }

        return m_romBanks[bankNumber];
    }

    const std::vector<uint8>& Cartridge::GetCHRBank(uint16 bankNumber)
    {
        if (bankNumber > m_romBanks.size())
        {
            throw("Unable to load bank from cartridge");
            return std::vector<uint8>();
        }

        return m_vramBanks[bankNumber];
    }
}
