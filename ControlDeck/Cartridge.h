// Copyright © Allan Moore April 2020
#pragma once

#include "Common.h"

namespace ControlDeck
{
	enum class CartridgeFlags : uint8
	{
		// 1 for vertical, 0 for horizontal 
		Mirroring = 0x1,
		// Battery backed ram at $6000-$7FFF 
		BatteryBackedRam = 0x2,
		// Trainer, 512byte trainer at $7000-$71FF
		Trainer = 0x4,
		// Four screen VRam layout
		FourScreenVRam = 0x8,
	};

	class Cartridge
	{
	public:
		Cartridge() = default;
		bool Load(std::string Path);
		const std::vector<ubyte>& GetPRGBank(uint16 bankNumber);
		const std::vector<ubyte>& GetCHRBank(uint16 bankNumber);

		int GetNumVRamBanks() const { return m_vramBanks.size(); }

	private:
		const uint16 m_romBankSize = 16384;
		const uint16 m_vramBankSize = 8192;

		bool m_bDoneLoad = false;
		uint8 m_prgRomBanks = 0;
		uint8 m_chrVRamBanks = 0;
		uint8 m_flags = 0;
		std::string m_path;

		std::vector<std::vector<ubyte>> m_romBanks;
		std::vector<std::vector<ubyte>> m_vramBanks;
	};
}