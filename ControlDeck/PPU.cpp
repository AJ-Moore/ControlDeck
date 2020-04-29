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
		if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
		{
			return false;
		}

		m_sdlWindow = SDL_CreateWindow("Control Deck", 100, 100, 256*3, 240*3, 0);
		m_sdlSurface = SDL_CreateRGBSurface(0,256, 240, 32, 0xff0000, 0x00ff00, 0x0000ff,0x0);
	}

	void PPU::Update()
	{
		SDL_PumpEvents();
		LoadRegistersFromCPU();

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
			Render();
		}

		IncrementCycle();
		WriteRegistersToCPU();
	}

	void putpixel(SDL_Surface* surface, int x, int y, Uint32 Pixel)
	{
		Uint8* pixel = (Uint8*)surface->pixels;
		pixel += (y * surface->pitch) + (x * sizeof(Uint32));
		*((Uint32*)pixel) = Pixel;
	}

	void PPU::Render()
	{
		SDL_LockSurface(m_sdlSurface);
		for (uint x = 0; x < 256; x++)
		{
			for (uint y = 0; y < 240; y++)
			{
				putpixel(m_sdlSurface, x, y, m_pixelBuffer[x + (y * 256)]);
			}
		}
		SDL_UnlockSurface(m_sdlSurface);

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
	}

	void PPU::LoadRegistersFromCPU()
	{
		m_ppuCTRL = m_cpu->ReadMemory8(PPU_CTRL_ADR);
		m_ppuMask = m_cpu->ReadMemory8(PPU_MASK_ADR);
		//m_ppuStatus = m_cpu->ReadMemory8(PPU_STATUS_ADR);
		m_oamAddr = m_cpu->ReadMemory8(OAM_ADR);
		//m_oamData = m_cpu->ReadMemory8(OAM_DATA_ADR);
		//m_ppuScroll = m_cpu->ReadMemory8(PPU_SCROLL_ADR);
		//m_ppuAddr = m_cpu->ReadMemory8(PPU_ADR);
		//m_ppuData = m_cpu->ReadMemory8(PPU_DATA_ADR);
		//m_oamDMA = m_cpu->ReadMemory8(OAM_DMA_ADR);
	}

	void PPU::WriteRegistersToCPU()
	{
		//m_cpu->WriteMemory8(PPU_CTRL_ADR, m_ppuCTRL);
		//m_cpu->WriteMemory8(PPU_MASK_ADR, m_ppuMask);
		//m_cpu->WriteMemory8(PPU_STATUS_ADR, m_ppuStatus);
		//m_cpu->WriteMemory8(OAM_ADR, m_oamAddr);
		//m_cpu->WriteMemory8(OAM_DATA_ADR, m_oamData);
		//m_cpu->WriteMemory8(PPU_SCROLL_ADR, m_ppuScroll);
		//m_cpu->WriteMemory8(PPU_ADR, m_ppuAddr);
		//m_cpu->WriteMemory8(PPU_DATA_ADR, m_ppuData);
		//m_cpu->WriteMemory8(OAM_DMA_ADR, m_oamDMA);
	}

	uint8 PPU::ReadMemory8(uint16 Addr)
	{
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
		uint8 spriteIndex = 0;
		uint8 spriteHeight = 8;
		uint8 spritePosY = 0;

		std::fill(m_secondaryOAM.begin(), m_secondaryOAM.end(), 0);

		if (m_ppuCTRL & (uint8)PPUCtrl::SpriteSize)
		{
			spriteHeight = 16;
		}

		for (uint16 i = 0; i <= 0xFF; i += 4)
		{
			spritePosY = m_primaryOAM[i];
			
			// hack for time being 
			if (spritePosY == 0)
			{
				continue;
			}

			if (spritePosY >= scanline && scanline <= spritePosY + spriteHeight)
			{
				if (spriteIndex >= 8)
				{
					// Set overflow and exit.
					SetPPUStatus(PPUStatus::SpriteOverflow, true);
					break;
				}

				// Load sprite into secondary OAM
				for (uint8 p = 0; p < 4; ++p)
				{
					m_secondaryOAM[(spriteIndex*4) + p] = m_primaryOAM[i + p];
				}
				spriteIndex++;
			}
		}
	}

	static int colour = 0;

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
				colour++;
			}
		}
	}

	void PPU::SetVblank()
	{
		if (m_currentScanline == 241 && m_currentCycle == 1)
		{
			SetPPUCtrl(PPUCtrl::VerticalBlanking, true);
			SetPPUStatus(PPUStatus::VerticalBlank, true);
		}
	}

	void PPU::ClearVblank()
	{
		if (m_currentScanline == 261 && m_currentCycle == 1)
		{
			SetPPUStatus(PPUStatus::VerticalBlank, false);
		}
	}

	uint8 PPU::GetNametableAddress()
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

		uint8 tileX = m_currentCycle / 32;
		uint8 tileY = m_currentScanline / 32;

		uint8 subTileX = 1-(m_currentCycle / 8) % 2;
		uint8 subTileY = 1-(m_currentScanline / 8) % 2;

		uint8 attributeTable = ReadMemory8(GetNametableAddress() + ATTRIB_OFFSET + (tileX + (tileY * 8)));
		uint8 paletteIndex = (attributeTable >> ((subTileX * 2) + (subTileY * 4))) & 0x3;

		// Each 'tile' in the pattern table is 16 bytes, index * 16 to get location in table
		for (int i = 0; i < 8; ++i)
		{
			uint16 address = patternAddress + (nameTableByte * 16) + i;

			// Write each pixel to buffer
			for (int p = 0;p < 8; ++p)
			{
				// get pixel left to right
				uint8 pixel1 = (ReadMemory8(address) >> (0x1 * 7 - p)) & 0x1;
				uint8 pixel2 = (ReadMemory8(address + 8) >> (0x1 * 7 - p)) & 0x1;
				uint8 pixel = (pixel2 << 0x1) | pixel1;

				// Write to buffer, scanline + i = y m_currentCycle + p = x
				uint posY = m_currentScanline + i;
				uint posX = m_currentCycle + p;

				uint pos = posX + (posY * 256);

				if (pos > m_pixelBuffer.size())
				{
					throw ("Out of range!");
					return;
				}

				m_pixelBuffer[pos] = PALETTE[pixel + (paletteIndex*3)];
			}
		}

	}

	void PPU::DrawSprites()
	{
		// Todo add sprite priority - front back sorting
		uint startScanline = m_currentCycle - 8;

		for (uint i = 0; i < 8; ++i)
		{
			uint scanline = startScanline + i;
			LoadSpritesForScanline(scanline);

			// For each of  the 8 sprites selected for this scanline
			for (uint p = 0; p < 8; ++p)
			{
				uint8 yPosition = m_secondaryOAM[p * 4];
				uint8 xPosition = m_secondaryOAM[(p * 4)+3];
				uint8 yOffset = yPosition - scanline;
				uint8 patternOffset = m_secondaryOAM[(p * 4) + 1];

				// if 8x8 sprite byte 1 indicates pattern table tile index
				if (m_ppuCTRL & (uint8)PPUCtrl::SpriteSize)
				{
					printf("Error: Unsupported sprite size");
					throw("DIE");
				}
				else
				{
					// get tile
					uint16 patternAddress = (m_ppuCTRL & (uint8)PPUCtrl::SpritePatternAddress) ? 0x1000 : 0x0;
					uint16 address = patternAddress + (patternOffset * 16) + i;

					// Write each pixel to buffer
					for (int q = 0; q < 8; ++q)
					{
						// get pixel left to right
						uint8 pixel1 = (ReadMemory8(address) >> (0x1 * 7 - q)) & 0x1;
						uint8 pixel2 = (ReadMemory8(address + 8) >> (0x1 * 7 - q)) & 0x1;
						uint8 pixel = (pixel2 << 0x1) | pixel1;

						// Write to buffer, scanline + i = y m_currentCycle + p = x
						uint posY = scanline + yOffset + i;
						uint posX = xPosition + q;

						uint pos = posX + (posY * 256);

						if (pos >= m_pixelBuffer.size())
						{
							//throw ("Out of range!");
							return;
						}

						m_pixelBuffer[pos] = PALETTE[pixel +2];
					}
					
				}

			}
		}
	}

	void PPU::VisibleScanline()
	{
		if (m_currentCycle == 0)
		{
			// PPU Address during cycle zero - same CHR address used to fetch background tile low bytes
		}

		if (m_currentCycle < 256)
		{
			m_currentTile = (m_currentCycle / 8) + ((m_currentScanline / 8) * 32);

			if ((m_currentCycle % 8) == 0 && m_currentScanline % 8 == 0)
			{
				DrawTile();
				if (m_currentCycle != 0)
				{
					DrawSprites();
				}
			}
		}

		/// OAMADDR is set to 0 257-320 of pre-render and visible scanlines
		if (m_currentCycle >= 257 && m_currentCycle <= 320)
		{
			m_oamAddr = 0;
			m_cpu->WriteMemory8(OAM_ADR, m_oamAddr);
			LoadRegistersFromCPU();
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
			m_oamAddr = 0;
			m_cpu->WriteMemory8(OAM_ADR, m_oamAddr);
			LoadRegistersFromCPU();
		}

		ClearVblank();
	}
}

