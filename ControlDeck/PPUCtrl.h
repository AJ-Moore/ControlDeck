#pragma once
#include "Common.h"

//	7  bit  0
//	---- ----
//	VPHB SINN
//	|||| ||||
//	|||| ||++ - Base nametable address
//	|||| ||    (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00)
//	|||| |+-- - VRAM address increment per CPU read / write of PPUDATA
//	|||| |     (0: add 1, going across; 1: add 32, going down)
//	|||| +----Sprite pattern table address for 8x8 sprites
//	||||       (0: $0000; 1: $1000; ignored in 8x16 mode)
//	|||+------Background pattern table address(0: $0000; 1: $1000)
//	|| +------ - Sprite size(0: 8x8 pixels; 1: 8x16 pixels)
//	| +--------PPU master / slave select
//	| (0: read backdrop from EXT pins; 1: output color on EXT pins)
//	+ -------- - Generate an NMI at the start of the
//	vertical blanking interval(0: off; 1: on)
enum class PPUCtrl : uint8
{
	// bit 0 & 1 used to indicate base nametable address (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00)
	NametableBit0 = 0x1,
	NametableBit1 = 0x2,

	// VRam address increment per CPU read/write of PPUData 0: +1 across 1: +32 down 1
	VRamAddressIncrement = 0x4,

	// Sprite pattern table addr 0: $0000 1:1000, ignored in 8x16
	SpritePatternAddress = 0x8,

	// Background pattern table addr 0: $0000 1: $1000
	BackgroundPatternAddress = 0x10,

	// Sprite size 0: 8x8 1:16x16
	SpriteSize = 0x20,

	// (0: read backdrop from EXT pins; 1: output color on EXT pins)
	PPUMasterSlaveSelect = 0x40,

	// Generate NMI at start of Vertical Blanking interval
	VerticalBlanking = 0x80
};
