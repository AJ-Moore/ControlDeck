// Copyright © Allan Moore April 2020

#include "Common.h"
#include "Cartridge.h"
#include "CPU.h"
#include "PPU.h"

using namespace ControlDeck;

int main()
{
    SharedPtr<Cartridge> mario = std::make_shared<Cartridge>();
    mario->Load(".\\mario.nes");

    SharedPtr<CPU> cpu = std::make_shared<CPU>();
    SharedPtr<PPU> ppu = std::make_shared<PPU>(cpu.get());
    cpu->SetPPU(ppu.get());
    
    if (!ppu->Init())
    {
        printf("PPU Initialisation failed!");
        return 0;
    }
    cpu->Init();

    // After initailisation load cartridge
    cpu->LoadCartridge(rom.get());

    bool bRunning = true;
    uint prevCPUCycle = 0; 

    while (bRunning)
    {
        for (uint i = 0; i < 15; ++i)
        {
            cpu->Update();
        }

        uint cycles = cpu->GetCPUCycles() - prevCPUCycle;
        prevCPUCycle = cpu->GetCPUCycles();

        // 1 cpu cycle = 3 ppu cycles, little hacky for time being.
        for (uint p = 0; p < cycles*3; ++p )
        {
            ppu->Update();

            if (ppu->GetPPUCycles() == 260)
            {
                break;
            }
        }
    }
}
