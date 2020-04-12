// Copyright © Allan Moore April 2020

#include "Common.h"
#include "Cartridge.h"
#include "CPU.h"

using namespace ControlDeck;

int main()
{
    SharedPtr<Cartridge> mario = std::make_shared<Cartridge>();
    mario->Load(".\\mario.nes");

    SharedPtr<CPU> cpu = std::make_shared<CPU>();
    cpu->LoadCartridge(mario.get());
    cpu->Update();
}
