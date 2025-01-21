#pragma once
#include "Common.h"

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