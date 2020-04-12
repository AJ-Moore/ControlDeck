#pragma once
#include "Common.h"

namespace ControlDeck
{
	enum PPUCtrlFlag
	{

	};


	class PPU
	{
	public:
		PPU() = default;
	private:
		// Copies memory mapped registers from CPU
		void UpdateRegisters();

		// CPU ADDRESS LOCATIONS
		// https://wiki.nesdev.com/w/index.php/PPU_registers#OAMADDR
		const uint16 PPU_CTRL_ADR = 0x2000;
		const uint16 PPU_MASK_ADR = 0x2001;
		const uint16 PPU_STATUS_ADR = 0x2002;
		const uint16 OAM_ADR_ADR = 0x2003;
		const uint16 OAM_DATA_ADR = 0x2004;
		const uint16 PPU_SCROLL_ADR = 0x2005;
		const uint16 PPU_ADR = 0x2006;
		const uint16 PPU_DATA_ADR = 0x2007;
		const uint16 OAM_DMA_ADR = 0x4014;

		uint8 m_ppuCTRL = 0;
		uint8 m_ppuMask = 0;
		uint8 m_ppuStatus = 0;
		uint8 m_oamAddr = 0;
		uint8 m_oamData = 0;
		uint8 m_ppuScroll = 0;
		uint8 m_ppuAddr = 0;
		uint8 m_ppuData = 0;
		uint8 m_oamDMA = 0;

	};
}