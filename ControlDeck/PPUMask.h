#pragma once

#include "Common.h"

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
enum class PPUMask : uint8
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