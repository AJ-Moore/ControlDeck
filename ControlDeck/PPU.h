#pragma once
#include "Common.h"
#include "PPUCtrl.h"

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

	//  7  bit  0
	//	---- ----
	//	BGRs bMmG
	//	|||| ||||
	//	|||| |||+-Greyscale(0: normal color, 1 : produce a greyscale display)
	//	|||| ||+-- 1: Show background in leftmost 8 pixels of screen, 0 : Hide
	//	|||| |+-- - 1 : Show sprites in leftmost 8 pixels of screen, 0 : Hide
	//	|||| +---- 1 : Show background
	//	|||+------ 1 : Show sprites
	//	|| +------ - Emphasize red
	//	| +--------Emphasize green
	//	+ -------- - Emphasize blue
	enum class Mask : uint8
	{
		// G - 0: normal colour, 1: greyscale
		Greyscale = 0x1,

		// m - 1: Show background in leftmost 8 pixels of screen 0: hide
		BackgroundLeftmost8Pixels = 0x2, 

		// M - 1: Show sprites in leftmost 8 pixels of screen 0: hide
		SpritesLeftmost8Pixels = 0x4,

		// b - Show background if set 
		ShowBackground = 0x8, 

		// s - Show sprites
		ShowSprites = 0x10, 

		// Emphasize red 
		EmphasizeRed = 0x20,

		// Emphasize green 
		EmphasizeGreen = 0x40,

		// Emphasize blue 
		EmphasizeBlue = 0x80,
	};

	class CPU;

	// 262 scanlines per frame 
	// 1 scaneline == 341 ppu clock cycles - 1CPU = 3 PPU
	class PPU
	{
		friend class CPU;
	public:
		PPU() = delete;
		PPU(CPU* cpu);

		bool Init();
		void Update();
		void Render();
		void WriteOAMByte(uint8 addr, uint8 data);
		void WriteMemory8(uint16 Addr, uint8 Data);

		uint GetPPUCycles() const { return m_currentCycle; }

	private:
		// Copies memory mapped registers between CPU <--> PPU 
		void LoadRegistersFromCPU();
		void WriteRegistersToCPU();

		uint8 ReadMemory8(uint16 Addr);


		void SetPPUStatus(PPUStatus, bool bEnabled);
		void SetPPUCtrl(PPUCtrl status, bool bEnabled);
		uint8 GetBackgroundPaletteIndex();

		// Loads 8 sprites for current scanline into secondary oam
		void LoadSpritesForScanline(uint scanline);

		void IncrementCycle();
		void SetVblank();
		void ClearVblank();
		uint8 GetNametableAddress();
		void DrawTile();
		void DrawSprites();
		void VisibleScanline();
		void PostRenderScanline();
		void PreRenderScanline();

		SDL_Window* m_sdlWindow = nullptr;
		SDL_Renderer* m_sdlRenderer = nullptr;
		SDL_Surface* m_sdlSurface = nullptr;

		// 16 kb address space used by ppu 
		std::vector<ubyte> m_vram;

		// 256 bytes OAM - Object arribute memory, holds 64 sprites, each sprite is 4 bytes
		std::vector<ubyte> m_primaryOAM;

		// 64 bytes OAM - holds 8 sprites for the current scanline.
		std::vector<ubyte> m_secondaryOAM;

		// Pixel buffer to be "blitted" to screen
		std::vector<uint> m_pixelBuffer;

		CPU* m_cpu = nullptr;
		uint m_currentCycle = 0;
		uint m_currentScanline = 0;
		uint m_currentTile = 0;

		// PPU RAM ADDRESS START LOCATIONS
		const uint16 NAMETABLE_ADR = 0x2000;
		const uint16 ATTRIB_OFFSET = 0x3C0;

		// CPU ADDRESS LOCATIONS
		// https://wiki.nesdev.com/w/index.php/PPU_registers#OAMADDR
		const uint16 PPU_CTRL_ADR = 0x2000;
		const uint16 PPU_MASK_ADR = 0x2001;
		const uint16 PPU_STATUS_ADR = 0x2002;
		const uint16 OAM_ADR = 0x2003;
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