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
    cpu->LoadCartridge(mario.get());
    
    SharedPtr<PPU> ppu = std::make_shared<PPU>(cpu.get());

    bool bRunning = true;

    while (bRunning)
    {
        cpu->Update();
        ppu->Update();
    }
}
