#include "PPU.h"

namespace ControlDeck
{
	PPU::PPU(CPU* cpu)
	{
		m_cpu = cpu;
		m_ram.resize(0x10000);
	}

	void PPU::Update()
	{
		LoadRegistersFromCPU();


		WriteRegistersToCPU();
	}

	void PPU::LoadRegistersFromCPU()
	{
		m_ppuCTRL = m_cpu->ReadMemory8(PPU_CTRL_ADR);
		m_ppuMask = m_cpu->ReadMemory8(PPU_MASK_ADR);
		m_ppuStatus = m_cpu->ReadMemory8(PPU_STATUS_ADR);
		m_oamAddr = m_cpu->ReadMemory8(OAM_ADR_ADR);
		m_oamData = m_cpu->ReadMemory8(OAM_DATA_ADR);
		m_ppuScroll = m_cpu->ReadMemory8(PPU_SCROLL_ADR);
		m_ppuAddr = m_cpu->ReadMemory8(PPU_ADR);
		m_ppuData = m_cpu->ReadMemory8(PPU_DATA_ADR);
		m_oamDMA = m_cpu->ReadMemory8(OAM_DMA_ADR);
	}

	void PPU::WriteRegistersToCPU()
	{
		m_cpu->WriteMemory8(PPU_CTRL_ADR, m_ppuCTRL);
		m_cpu->WriteMemory8(PPU_MASK_ADR, m_ppuMask);
		m_cpu->WriteMemory8(PPU_STATUS_ADR, m_ppuStatus);
		m_cpu->WriteMemory8(OAM_ADR_ADR, m_oamAddr);
		m_cpu->WriteMemory8(OAM_DATA_ADR, m_oamData);
		m_cpu->WriteMemory8(PPU_SCROLL_ADR, m_ppuScroll);
		m_cpu->WriteMemory8(PPU_ADR, m_ppuAddr);
		m_cpu->WriteMemory8(PPU_DATA_ADR, m_ppuData);
		m_cpu->WriteMemory8(OAM_DMA_ADR, m_oamDMA);
	}

	uint8 PPU::ReadMemory8(uint16 Addr)
	{
		return m_ram[Addr];
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
			SetPPUStatus(PPUStatus::VerticalBlank, true);
			m_currentTile = 0;
		}
	}

	void PPU::ClearVblank()
	{
		if (m_currentScanline == 261 && m_currentCycle == 1)
		{
			SetPPUStatus(PPUStatus::VerticalBlank, false);
		}
	}

	void PPU::FetchTile()
	{
		//Load nametable byte 
		uint8 nameTable = ReadMemory8(NAMETABLE_ADR + m_currentTile);
		uint8 attributeTable = ReadMemory8(ATTRIB_ADR + (m_currentTile / 4));

		m_currentTile++;
	}

	void PPU::VisibleScanline()
	{
		if (m_currentCycle <= 256)
		{
			if ((m_currentCycle % 8) == 0)
			{
				FetchTile();
			}
		}

		IncrementCycle();
	}

	void PPU::PostRenderScanline()
	{
		SetVblank();
		IncrementCycle();
	}

	void PPU::PreRenderScanline()
	{
		ClearVblank();
	}
}

