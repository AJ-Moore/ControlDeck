// Copyright © Allan Moore April 2020

#include "Common.h"
#include "Cartridge.h"
#include "CPU.h"
#include "PPU.h"
#include "APU.h"
#include "WaveformGenerator.h"

using namespace ControlDeck;

int main()
{
    SharedPtr<Cartridge> rom = std::make_shared<Cartridge>();
    //rom->Load(".\\mario.nes");
    //rom->Load(".\\scroll.nes");
    //rom->Load(".\\roms\\Rockman.nes");
    //rom->Load(".\\roms\\Millipede.nes");
    rom->Load(".\\kong.nes");

    SharedPtr<CPU> cpu = std::make_shared<CPU>();
    SharedPtr<PPU> ppu = std::make_shared<PPU>(cpu.get());
    SharedPtr<APU> apu = std::make_shared<APU>(cpu.get());
    cpu->SetPPU(ppu.get());
    apu->Init();
    
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
    uint cycles = 0;
    double previousTimeElapsed = SDL_GetPerformanceCounter();
    double frameTime = 1.0l / 60.0l;

    uint32_t last_tick_time = 0;
    uint32_t delta = 0;

    //WaveformGenerator wave;
    //wave.Init();

    while (bRunning)
    {
        for (uint i = 0; i < 30; ++i)
        {
            cpu->Update();
            apu->Update();
        }

        cycles = cpu->GetCPUCycles() - prevCPUCycle;
        prevCPUCycle = cpu->GetCPUCycles();

        // 1 cpu cycle = 3 ppu cycles, little hacky for time being.
        for (uint p = 0; p < cycles * 3; ++p )
        {
            ppu->Update();

            if (ppu->GetPPUCycles() == 260)
            {
               break;
            }
        }

        if (cpu->GetCPUCycles() >= 29780)
        {
            double deltaTime = (double)(SDL_GetPerformanceCounter() - previousTimeElapsed) / (double)SDL_GetPerformanceFrequency();
            previousTimeElapsed = SDL_GetPerformanceCounter();

            if (deltaTime < frameTime)
            {
                SDL_Delay((frameTime - deltaTime)*1000);
            }

            cpu->ResetCPUCycles();
            //prevCPUCycle = 0;
        }
    }
}
