#include "PPU.h"
#include "CPU.h"
#include "PPUCtrl.h"
#include "Palette.h"

namespace ControlDeck
{
	PPU::PPU(CPU* cpu)
	{
		m_cpu = cpu;
		m_vram.resize(0x10000);
		m_primaryOAM.resize(0x100);
		m_secondaryOAM.resize(0x20);
		m_pixelBuffer.resize(256 * 240);
	}

	bool PPU::Init()
	{
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
		{
			printf("SDL initialisation failed!");
			return false;
		}
		
		m_sdlWindow = SDL_CreateWindow("Control Deck", 0, 0, 256*3, 240*3, SDL_WINDOW_RESIZABLE);
		m_sdlSurface = SDL_CreateRGBSurface(0,256, 240, 32, 0xff0000, 0x00ff00, 0x0000ff,0x0);
		SDL_FillRect(m_sdlSurface, NULL, 0x000000);
		SDL_UpdateWindowSurface(m_sdlWindow);
		return true;
	}

	uint8 buffer = 0;

	void PPU::Update()
	{
		//LoadRegistersFromCPU();

		if (m_currentScanline < 240)
		{
			VisibleScanline();
		}
		else if (m_currentScanline < 261)
		{
			PostRenderScanline();
		}
		else
		{
			PreRenderScanline();
		}

		if (m_currentScanline == 260 && m_currentCycle == 0)
		{
			SDL_PumpEvents();
			m_cpu->UpdateInput();
			Render();
		}

		IncrementCycle();
	}

	void putpixel(SDL_Surface* surface, int x, int y, Uint32 Pixel)
	{
		Uint8* pixel = (Uint8*)surface->pixels;
		pixel += (y * surface->pitch) + (x * sizeof(Uint32));
		*((Uint32*)pixel) = Pixel;
	}

	void PPU::Render()
	{
		SDL_memcpy(m_sdlSurface->pixels, &m_pixelBuffer[0], sizeof(Uint32) * m_pixelBuffer.size());
		SDL_BlitScaled(m_sdlSurface, nullptr, SDL_GetWindowSurface(m_sdlWindow), nullptr);
		SDL_UpdateWindowSurface(m_sdlWindow);
	}

	void PPU::WriteOAMByte(uint8 addr, uint8 data)
	{
		m_primaryOAM[addr] = data;
	}

	void PPU::WriteMemory8(uint16 Addr, uint8 Data)
	{
		m_vram[Addr] = Data;

		// if vertical mirroring, : $2000 equals $2800 and $2400 equals $2C00 
		if (true)
		{	
			if (Addr >= 0x2000 && Addr <= 0x23ff)
			{
				m_vram[Addr + 0x800] = Data;
			}

			if (Addr >= 0x2800 && Addr <= 0x2bff)
			{
				m_vram[Addr - 0x800] = Data;
			}

			if (Addr >= 0x2400 && Addr <= 0x27ff)
			{
				m_vram[Addr + 0x800] = Data;
			}

			if (Addr >= 0x2C00 && Addr <= 0x2fff)
			{
				m_vram[Addr - 0x800] = Data;
			}

		}

		//$3000 - $3EFF	$0F00	Mirrors of $2000 - $2EFF
		if (Addr >= 0x3000 && Addr <= 0x3EFF)
		{
			m_vram[Addr - 0x1000] = Data;
		}

		if (Addr >= 0x2000 && Addr < 0x2EFF)
		{
			m_vram[Addr + 0x1000] = Data;
		}

		// Addresses $3F10/$3F14/$3F18/$3F1C are mirrors of $3F00/$3F04/$3F08/$3F0C
		if (Addr == 0x3F10 || Addr == 0x3F00)
		{
			m_vram[0x3F10] = Data;
			m_vram[0x3F00] = Data;
		}

		else if (Addr == 0x3F14 || Addr == 0x3F04)
		{
			m_vram[0x3F14] = Data;
			m_vram[0x3F04] = Data;
		}

		else if (Addr == 0x3F18 || Addr == 0x3F08)
		{
			m_vram[0x3F18] = Data;
			m_vram[0x3F08] = Data;
		}

		else if (Addr == 0x3F1C || Addr == 0x3F0C)
		{
			m_vram[0x3F1C] = Data;
			m_vram[0x3F0C] = Data;
		}
	}

	void PPU::LoadRegistersFromCPU()
	{
		m_ppuCTRL = m_cpu->RAM[PPU_CTRL_ADR];
		m_ppuMask = m_cpu->RAM[PPU_MASK_ADR];
		m_oamAddr = m_cpu->RAM[OAM_ADR];
	}

	uint8 PPU::ReadMemory8(uint16 Addr, bool memoryMappedIO)
	{
		if (Addr <= 0X3EFF )
		{
			uint data = buffer;
			buffer = m_vram[Addr];

			if (memoryMappedIO)
			{
				return data;
			}
		}

		return m_vram[Addr];
	}

	void PPU::SetPPUCtrl(PPUCtrl status, bool bEnabled)
	{
		if (bEnabled)
		{
			m_ppuCTRL |= (uint8)status;
		}
		else
		{
			m_ppuCTRL &= ~(uint8)status;
		}

		m_cpu->WriteMemory8(PPU_CTRL_ADR, m_ppuCTRL);
	}

	void PPU::SetPPUStatus(PPUStatus status, bool bEnabled)
	{
		if (bEnabled)
		{
			m_ppuStatus |= (uint8)status;
		}
		else
		{
			m_ppuStatus &= ~(uint8)status;
		}

		m_cpu->WriteMemory8(PPU_STATUS_ADR, m_ppuStatus);
	}

	uint8 PPU::GetBackgroundPaletteIndex()
	{
		// If bit 6 of PPUCTRL is clear the ppu gets the palette index for background from EXT pins, stock NES grounds these pins resulting in 0
		if ((m_ppuCTRL & (uint8)PPUCtrl::PPUMasterSlaveSelect) == 0)
		{
			return 0;
		}

		return uint8();
	}

	void PPU::LoadSpritesForScanline(uint scanline )
	{
		uint8 spriteHeight = 8;
		uint8 spritePosY = 0;

		std::fill(m_secondaryOAM.begin(), m_secondaryOAM.end(), 0);
		m_totalSprites = 0;

		if (m_ppuCTRL & (uint8)PPUCtrl::SpriteSize)
		{
			spriteHeight = 16;
		}

		for (uint16 i = 0; i <= 0xFF; i += 4)
		{
			spritePosY = m_primaryOAM[i];

			if (std::abs((int)spritePosY - (int)scanline) <= 8)
			{
				if (m_totalSprites >= 8)
				{
					// Set overflow and exit.
					SetPPUStatus(PPUStatus::SpriteOverflow, true);
					break;
				}

				// Load sprite into secondary OAM
				for (uint8 p = 0; p < 4; ++p)
				{
					m_secondaryOAM[(m_totalSprites * 4) + p] = m_primaryOAM[i + p];
				}
				m_totalSprites++;
			}
		}
	}

	void PPU::IncrementCycle()
	{
		m_currentCycle++;

		if (m_currentCycle > 340)
		{
			m_currentCycle = 0;
			m_currentScanline++;

			if (m_currentScanline > 261)
			{
				m_currentScanline = 0;
			}
		}
	}

	void PPU::SetVblank()
	{
		if (m_currentScanline == 241 && m_currentCycle == 1)
		{
			/// OAMADDR is set to 0 257-320 of pre-render and visible scanlines
			m_oamAddr = 0;
			m_cpu->WriteMemory8(OAM_ADR, m_oamAddr);
			LoadRegistersFromCPU();


			//SetPPUCtrl(PPUCtrl::VerticalBlanking, true);
			SetPPUStatus(PPUStatus::VerticalBlank, true);
			m_cpu->setNMI(true);
		}
	}

	void PPU::ClearVblank()
	{
		if (m_currentScanline == 261 && m_currentCycle == 1)
		{
			//SetPPUStatus(PPUStatus::VerticalBlank, false);
			m_cpu->setNMI(false);
		}
	}

	uint16 PPU::GetNametableAddress()
	{
		uint8 nametable = m_ppuCTRL & 0x3;

		switch (nametable)
		{
			case 0: return 0x2000;
			case 1: return 0x2400;
			case 2: return 0x2800;
			case 3: return 0x2C00;
		}
		
		return 0x2000;
	}

	void PPU::DrawTile()
	{
		//Load nametable byte, nametable byte holds index into pattern table
		uint8 nameTableByte = ReadMemory8(GetNametableAddress() + m_currentTile);
		uint16 patternAddress = (m_ppuCTRL & (uint8)PPUCtrl::BackgroundPatternAddress) ? 0x1000 : 0x0;

		uint8 tileX = (m_currentCycle / 32);
		uint8 tileY = m_currentScanline / 32;
		
		uint8 subTileX = (m_currentCycle / 8) % 4 < 2 ? 0 : 1;
		uint8 subTileY = (m_currentScanline / 8) % 4 < 2 ? 0 : 1;

		uint8 attributeTable = ReadMemory8(GetNametableAddress() + ATTRIB_OFFSET + (tileX + (tileY * 8)));
		uint8 paletteIndex = (attributeTable >> ((subTileX * 2) + (subTileY * 4))) & 0x3;

		uint8 pixel, pixel1, pixel2, posX, posY;

		// Each 'tile' in the pattern table is 16 bytes, index * 16 to get location in table
		for (int i = 0; i < 8; ++i)
		{
			uint16 address = patternAddress + (nameTableByte * 16) + i;
			uint8 pixelMem1 = ReadMemory8(address);
			uint8 pixelMem2 = ReadMemory8(address + 8);

			// Write each pixel to buffer
			for (int p = 0;p < 8; ++p)
			{
				// get pixel left to right
				pixel1 = (pixelMem1 >> (0x1 * 7 - p)) & 0x1;
				pixel2 = (pixelMem2 >> (0x1 * 7 - p)) & 0x1;
				pixel = (pixel2 << 0x1) | pixel1;

				// Write to buffer, scanline + i = y m_currentCycle + p = x
				posY = m_currentScanline + i + m_scrollY;
				posX = m_currentCycle + p + m_scrollX;

				if (posX >= 256)
				{
					continue;
				}

				if (posY >= 240)
				{
					continue;
				}

				uint pos = posX + (posY * 256);	
				uint8 colour = 0;

				if (m_ppuMask & (uint8)PPUMask::EmphasizeRed)
				{
					throw("");
				}

				if (m_ppuMask & (uint8)PPUMask::EmphasizeGreen)
				{
					throw("");
				}

				if (m_ppuMask & (uint8)PPUMask::EmphasizeBlue)
				{
					throw("");
				}

				if (m_ppuMask & (uint8)PPUMask::Greyscale)
				{
					colour &= 0x30;
				}
				
				if (pixel == 0)
				{
					m_pixelBuffer[pos] = PALETTE[m_vram[PALETTE_ADR]];
					//putpixel(m_sdlSurface, posX, posY, PALETTE[m_vram[PALETTE_ADR]]);
				}
				else
				{
					m_pixelBuffer[pos] = PALETTE[m_vram[PALETTE_ADR + (paletteIndex*4) + pixel]];
					//putpixel(m_sdlSurface, posX, posY, PALETTE[m_vram[PALETTE_ADR + (paletteIndex * 4) + pixel]]);
				}
			}
		}

	}

	void PPU::DrawSprites()
	{
		// draw 8 previous scanlines
		for (uint scanline = 0; scanline < 8; scanline++)
		{
			uint currentScanline = m_currentScanline - 8 + scanline;
			//uint currentScanline = m_currentScanline ;

			// Todo add sprite priority - front back sorting
			LoadSpritesForScanline(currentScanline);

			// For each of the total number of sprites up to 8, sprites selected for this scanline
			for (uint p = 0; p < m_totalSprites; ++p)
			{
				uint8 yPosition = m_secondaryOAM[p * 4] + 1;
				uint8 xPosition = m_secondaryOAM[(p * 4) + 3];
				uint8 patternOffset = m_secondaryOAM[(p * 4) + 1];
				uint8 paletteIndex = m_secondaryOAM[(p * 4) + 2] & 0x3;
				uint8 yOffset = currentScanline - yPosition ;

				bool flipX = m_secondaryOAM[(p * 4) + 2] & 0x40;
				bool flipY = m_secondaryOAM[(p * 4) + 2] & 0x80;

				if (yOffset < 0)
				{
					continue;
				}

				// if 8x8 sprite byte 1 indicates pattern table tile index
				if (m_ppuCTRL & (uint8)PPUCtrl::SpriteSize)
				{
					printf("Error: Unsupported sprite size 16x");
					throw("DIE");
				}
				else
				{

					// get tile
					uint16 patternAddress = (m_ppuCTRL & (uint8)PPUCtrl::SpritePatternAddress) ? 0x1000 : 0x0;
					uint16 address = patternAddress + (patternOffset * 16) + scanline;
					//uint16 address = patternAddress + (patternOffset * 16) + (m_currentScanline % 8);

					// Write each pixel to buffer
					for (int q = 0; q < 8; ++q)
					{
						uint pixelOffset = (flipX) ? 8 - q : q;

						// get pixel left to right
						uint8 pixel1 = (ReadMemory8(address) >> (0x1 * 7 - q)) & 0x1;
						uint8 pixel2 = (ReadMemory8(address + 8) >> (0x1 * 7 - q)) & 0x1;
						uint8 pixel = (pixel2 << 0x1) | pixel1;

						// Write to buffer, scanline + i = y m_currentCycle + p = x
						//uint posY = scanline + yOffset + i;
						uint posY = yPosition + scanline;
						//uint posY = yPosition + (m_currentScanline % 8);
						uint posX = xPosition + pixelOffset;

						uint pos = posX + (posY * 256);

						if (pixel == 0)
						{
							//m_pixelBuffer[pos] = PALETTE[m_vram[PALETTE_ADR]];
						}
						else
						{
							if (m_pixelBuffer[pos] != 0)
							{
								SetPPUStatus(PPUStatus::Sprite0Hit, true);
							}

							m_pixelBuffer[pos] = PALETTE[m_vram[PALETTE_ADR + ((4 + paletteIndex) * 4) + pixel]];
							//putpixel(m_sdlSurface, posX, posY, m_pixelBuffer[pos]);
						}
					}
					

				}

			}
		}

		
	}

	void PPU::VisibleScanline()
	{
		if (m_currentCycle == 256)
		{
			if (m_currentScanline == 0)
			{
				return;
			}

			if (m_currentScanline % 8 == 0)
			{
				DrawSprites();
			}
		}

		if (m_currentCycle < 256)
		{
			if ((m_currentCycle % 8) == 0 && m_currentScanline % 8 == 0)
			{
				m_currentTile = (m_currentCycle / 8) + ((m_currentScanline / 8) * 32);
				m_currentTile += m_coarseX;
				m_currentTile += m_coarseY * 32;
				DrawTile();
			}
		}	
		else //(m_currentCycle >= 257 && m_currentCycle <= 320)
		{
			/// OAMADDR is set to 0 257-320 of pre-render and visible scanlines
			//m_oamAddr = 0;
			//m_cpu->WriteMemory8(OAM_ADR, m_oamAddr);
			//LoadRegistersFromCPU();
		}
	}

	void PPU::PostRenderScanline()
	{
		SetVblank();
	}

	void PPU::PreRenderScanline()
	{
		/// OAMADDR is set to 0 257-320 of pre-render and visible scanlines
		if (m_currentCycle >= 257 && m_currentCycle <= 320)
		{
			//m_oamAddr = 0;
			//m_cpu->WriteMemory8(OAM_ADR, m_oamAddr);
			//LoadRegistersFromCPU();
		}

		ClearVblank();
	}
}

