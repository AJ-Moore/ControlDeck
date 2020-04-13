#pragma once
#include "Common.h"
#include "CPU.h"

//PPU Memory
//Address range	Size	Description
//$0000 - $0FFF	$1000	Pattern table 0
//$1000 - $1FFF	$1000	Pattern table 1
//$2000 - $23FF	$0400	Nametable 0
//$2400 - $27FF	$0400	Nametable 1
//$2800 - $2BFF	$0400	Nametable 2
//$2C00 - $2FFF	$0400	Nametable 3
//$3000 - $3EFF	$0F00	Mirrors of $2000 - $2EFF
//$3F00 - $3F1F	$0020	Palette RAM indexes
//$3F20 - $3FFF	$00E0	Mirrors of $3F00 - $3F1F


namespace ControlDeck
{
	enum PPUCtrl
	{

	};

	// 7  bit  0
	// ---- ----
	// VSO. ....
	enum class PPUStatus : uint8
	{
		// O - Sprite Overflow - set if more that 8 sprites appear on scanline (apparently buggy clared dot 1 of pre-render)
		SpriteOverflow = 0x20,

		// S - Sprite 0 hit - set if nonzero pixel of sprite 0 overlaps non zero background pixel (cleared at dot 1 of pre-render)
		Sprite0Hit = 0x40,

		// V - Vertical blank has started 0: nope 1: yes (set at dot 1 line 241 (post render), cleared after reading $2002 and dot 1 pre-render)
		VerticalBlank = 0x80,
	};

	// 262 scanlines per frame 
	// 1 scaneline == 341 ppu clock cycles - 1CPU = 3 PPU
	class PPU
	{
	public:
		PPU() = delete;
		PPU(CPU* cpu);

		void Update();

	private:
		// Copies memory mapped registers between CPU <--> PPU 
		void LoadRegistersFromCPU();
		void WriteRegistersToCPU();

		uint8 ReadMemory8(uint16 Addr);

		void SetPPUStatus(PPUStatus, bool bEnabled);

		void IncrementCycle();
		void SetVblank();
		void ClearVblank();
		void FetchTile();
		void VisibleScanline();
		void PostRenderScanline();
		void PreRenderScanline();

		// 16 kb address space used by ppu 
		std::vector<ubyte> m_ram;

		// 256 bytes OAM - Object arribute memory
		std::vector<ubyte> m_oam;

		// Pixel buffer to be "blitted" to screen
		std::vector<ubyte> m_pixelBuffer;

		CPU* m_cpu = nullptr;
		uint m_currentCycle = 0;
		uint m_currentScanline = 0;
		uint m_currentTile = 0;

		// PPU RAM ADDRESS LOCATIONS
		const uint16 NAMETABLE_ADR = 0x2000;
		const uint16 ATTRIB_ADR = 0x23C0;

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