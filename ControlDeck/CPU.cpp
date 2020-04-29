// Copyright © Allan Moore April 2020
// Author Allan Moore 20/ 03/ 2015 - April 2020

#include "CPU.h"
#include "PPUCtrl.h"
#include "AddressingMode.h"

namespace ControlDeck {

	CPU::CPU()
	{
		//!< The 6502 uses a 16 bit address bus $0 - $FFFF ($FFFF+1 possible addresses) 
		RAM.resize(0x10000); //!< err $FFFF invalid, wrap to 0?

		//!< Initialise RAM to NAUGHT
		std::fill(RAM.begin(), RAM.end(), 0);
	}

	void CPU::Init() {

	}

	void CPU::CheckForInterrupt()
	{
		if (ProcessorStatus & (uint8)PFlags::INTERRUPT_DISABLED)
		{
			return;
		}

		uint8 ppuCtrl = ReadMemory8(PPU_CTRL_ADR);

		if (ppuCtrl & (uint8)PPUCtrl::VerticalBlanking)
		{
			PushStack8(ProcessorStatus);
			PushStack16(PC);
			ppuCtrl &= ~(uint8)PPUCtrl::VerticalBlanking;
			WriteMemory8(PPU_CTRL_ADR, ppuCtrl);
			PC = ReadMemory16(0xFFFA);
		}
	}

	void CPU::DebugOutput()
	{
		uint8 opCode = ReadMemory8(PC);
		uint8 byte1 = ReadMemory8(PC + 1);
		uint8 byte2 = ReadMemory8(PC + 2);

		uint8 mode = (opCode >> 2) & 0x4;
		uint8 cc = opCode & 0x3;

		AdrMode adrMode = AdrMode::NONE;

		if (cc == 01)
		{
			switch (mode)
			{
			case 0x0: adrMode = AdrMode::ZERO_PAGEX; break;
			case 0x1: adrMode = AdrMode::ZERO_PAGE; break;
			case 0x2: adrMode = AdrMode::IMMEDIATE; break;
			case 0x3: adrMode = AdrMode::ABSOLUTE; break;
			case 0x4: adrMode = AdrMode::ZERO_PAGEY; break;
			case 0x5: adrMode = AdrMode::ZERO_PAGEX; break;
			case 0x6: adrMode = AdrMode::ABSOLUTEY; break;
			case 0x7: adrMode = AdrMode::ABSOLUTEX; break;
			}
		}
		else if (cc == 10)
		{
			switch (mode)
			{
			case 0x0: adrMode = AdrMode::IMMEDIATE; break;
			case 0x1: adrMode = AdrMode::ZERO_PAGE; break;
			case 0x2: adrMode = AdrMode::ACCUMULATOR; break;
			case 0x3: adrMode = AdrMode::ABSOLUTE; break;
			case 0x4: adrMode = AdrMode::ZERO_PAGEX; break;
			case 0x7: adrMode = AdrMode::ABSOLUTEY; break;
			}
		}
		else if (cc == 00)
		{
			switch (mode)
			{
			case 0x0: adrMode = AdrMode::IMMEDIATE; break;
			case 0x1: adrMode = AdrMode::ZERO_PAGE; break;
			case 0x3: adrMode = AdrMode::ABSOLUTE; break;
			case 0x5: adrMode = AdrMode::ZERO_PAGEX; break;
			case 0x7: adrMode = AdrMode::ABSOLUTEX; break;
			}
		}

		switch (adrMode)
		{
			case AdrMode::ABSOLUTE:
			case AdrMode::ABSOLUTEX:
			case AdrMode::ABSOLUTEY:
			case AdrMode::ABSOLUTE_INDIRECT: 
			case AdrMode::INDEXED_INDIRECT: 
				printf("%04X %02X %02X %02X\t\t", PC, opCode, byte1, byte2);
				break;
			case AdrMode::ZERO_PAGE:
			case AdrMode::ZERO_PAGEX: 
			case AdrMode::ZERO_PAGEY:
			case AdrMode::IMMEDIATE: 
			case AdrMode::INDIRECT_INDEXED: 
				printf("%04X %02X %02X   \t\t", PC, opCode, byte1);
				break;
		default:
			printf("%04X %02X      \t\t", PC, opCode);
		}

		printf("A:%02X X:%02X Y:%02X P:%02X SP:%02X\n", Accumulator, XReg, YReg, ProcessorStatus, SP);
	}

	void CPU::Update()
	{
		// Check for non-maskable interrupt
		CheckForInterrupt();
		uint8 opCode = ReadMemory8(PC);

		DebugOutput();

		switch (opCode) {
		case 0x00: BRK_$00(); break; case 0x18: CLC_$18(); break; case 0xD8: CLD_$D8(); break;
		case 0x58: CLI_$58(); break; case 0xB8: CLV_$B8(); break; case 0xCA: DEX_$CA(); break;
		case 0x88: DEY_$88(); break; case 0xE8: INX_$E8(); break; case 0xC8: INY_$C8(); break;
		case 0xEA: NOP_$EA(); break; case 0x48: PHA_$48(); break; case 0x08: PHP_$08(); break;
		case 0x68: PLA_$68(); break; case 0x28: PLP_$28(); break; case 0x40: RTI_$40(); break;
		case 0x60: RTS_$60(); break; case 0x38: SEC_$38(); break; case 0xf8: SED_$f8(); break;
		case 0x78: SEI_$78(); break; case 0xAA: TAX_$AA(); break; case 0xA8: TAY_$A8(); break;
		case 0xBA: TSX_$BA(); break; case 0x8A: TXA_$8A(); break; case 0x9A: TXS_$9A(); break;
		case 0x98: TYA_$98(); break; case 0x20: JSR_$20(); break;
		case 0x69: ADC(AdrMode::IMMEDIATE); break;
		case 0x65: ADC(AdrMode::ZERO_PAGE); break;
		case 0x75: ADC(AdrMode::ZERO_PAGEX); break;
		case 0x6D: ADC(AdrMode::ABSOLUTE); break;
		case 0x7D: ADC(AdrMode::ABSOLUTEX); break;
		case 0x79: ADC(AdrMode::ABSOLUTEY); break;
		case 0x61: ADC(AdrMode::INDEXED_INDIRECT); break;
		case 0x71: ADC(AdrMode::INDIRECT_INDEXED); break;
		case 0x29: AND(AdrMode::IMMEDIATE); break;
		case 0x25: AND(AdrMode::ZERO_PAGE); break;
		case 0x35: AND(AdrMode::ZERO_PAGEX); break;
		case 0x2D: AND(AdrMode::ABSOLUTE); break;
		case 0x3D: AND(AdrMode::ABSOLUTEX); break;
		case 0x39: AND(AdrMode::ABSOLUTEY); break;
		case 0x21: AND(AdrMode::INDEXED_INDIRECT); break;
		case 0x31: AND(AdrMode::INDIRECT_INDEXED); break;
		case 0x0A: ASL(AdrMode::ACCUMULATOR); break;
		case 0x06: ASL(AdrMode::ZERO_PAGE); break;
		case 0x16: ASL(AdrMode::ZERO_PAGEX); break;
		case 0x0E: ASL(AdrMode::ABSOLUTE); break;
		case 0x1E: ASL(AdrMode::ABSOLUTEX); break;
		case 0x90: BCC_$90(AdrMode::RELATIVE); break;
		case 0xB0: BCS_$B0(AdrMode::RELATIVE); break;
		case 0xF0: BEQ_$F0(AdrMode::RELATIVE); break;
		case 0x30: BMI_$30(AdrMode::RELATIVE); break;
		case 0xD0: BNE_D0(AdrMode::RELATIVE); break;
		case 0x10: BPL_$10(AdrMode::RELATIVE); break;
		case 0x50: BVC_$50(AdrMode::RELATIVE); break;
		case 0x70: BVS_$70(AdrMode::RELATIVE); break;
		case 0x24: BIT(AdrMode::ZERO_PAGE); break;
		case 0x2C: BIT(AdrMode::ABSOLUTE); break;
		case 0xC9: CMP(AdrMode::IMMEDIATE); break;
		case 0xC5: CMP(AdrMode::ZERO_PAGE); break;
		case 0xD5: CMP(AdrMode::ZERO_PAGEX); break;
		case 0xCD: CMP(AdrMode::ABSOLUTE); break;
		case 0xDD: CMP(AdrMode::ABSOLUTEX); break;
		case 0xD9: CMP(AdrMode::ABSOLUTEY); break;
		case 0xC1: CMP(AdrMode::INDEXED_INDIRECT); break;
		case 0xD1: CMP(AdrMode::INDIRECT_INDEXED); break;
		case 0xE0: CPX(AdrMode::IMMEDIATE); break;
		case 0xE4: CPX(AdrMode::ZERO_PAGE); break;
		case 0xEC: CPX(AdrMode::ABSOLUTE); break;
		case 0xC0: CPY(AdrMode::IMMEDIATE); break;
		case 0xC4: CPY(AdrMode::ZERO_PAGE); break;
		case 0xCC: CPY(AdrMode::ABSOLUTE); break;
		case 0xC6: DEC(AdrMode::ZERO_PAGE); break;
		case 0xD6: DEC(AdrMode::ZERO_PAGEX); break;
		case 0xCE: DEC(AdrMode::ABSOLUTE); break;
		case 0xDE: DEC(AdrMode::ABSOLUTEX); break;
		case 0x49: EOR(AdrMode::IMMEDIATE); break;
		case 0x45: EOR(AdrMode::ZERO_PAGE); break;
		case 0x55: EOR(AdrMode::ZERO_PAGEX); break;
		case 0x4D: EOR(AdrMode::ABSOLUTE); break;
		case 0x5D: EOR(AdrMode::ABSOLUTEX); break;
		case 0x59: EOR(AdrMode::ABSOLUTEY); break;
		case 0x41: EOR(AdrMode::INDEXED_INDIRECT); break;
		case 0x51: EOR(AdrMode::INDIRECT_INDEXED); break;
		case 0xE6: INC(AdrMode::ZERO_PAGE); break;
		case 0xF6: INC(AdrMode::ZERO_PAGEX); break;
		case 0xEE: INC(AdrMode::ABSOLUTE); break;
		case 0xFE: INC(AdrMode::ABSOLUTEX); break;
		case 0x4C: JMP(AdrMode::ABSOLUTE); break;
		case 0x6C: JMP(AdrMode::ABSOLUTE_INDIRECT); break;
		case 0xA9: LDA(AdrMode::IMMEDIATE); break;
		case 0xA5: LDA(AdrMode::ZERO_PAGE); break;
		case 0xB5: LDA(AdrMode::ZERO_PAGEX); break;
		case 0xAD: LDA(AdrMode::ABSOLUTE); break;
		case 0xBD: LDA(AdrMode::ABSOLUTEX); break;
		case 0xB9: LDA(AdrMode::ABSOLUTEY); break;
		case 0xA1: LDA(AdrMode::INDEXED_INDIRECT); break;
		case 0xB1: LDA(AdrMode::INDIRECT_INDEXED); break;
		case 0xA2: LDX(AdrMode::IMMEDIATE); break;
		case 0xA6: LDX(AdrMode::ZERO_PAGE); break;
		case 0xB6: LDX(AdrMode::ZERO_PAGEY); break;
		case 0xAE: LDX(AdrMode::ABSOLUTE); break;
		case 0xBE: LDX(AdrMode::ABSOLUTEY); break;
		case 0xA0: LDY(AdrMode::IMMEDIATE); break;
		case 0xA4: LDY(AdrMode::ZERO_PAGE); break;
		case 0xB4: LDY(AdrMode::ZERO_PAGEX); break;
		case 0xAC: LDY(AdrMode::ABSOLUTE); break;
		case 0xBC: LDY(AdrMode::ABSOLUTEX); break;
		case 0x4A: LSR(AdrMode::ACCUMULATOR); break;
		case 0x46: LSR(AdrMode::ZERO_PAGE); break;
		case 0x56: LSR(AdrMode::ZERO_PAGEX); break;
		case 0x4E: LSR(AdrMode::ABSOLUTE); break;
		case 0x5E: LSR(AdrMode::ABSOLUTEX); break;
		case 0x09: ORA(AdrMode::IMMEDIATE); break;
		case 0x05: ORA(AdrMode::ZERO_PAGE); break;
		case 0x15: ORA(AdrMode::ZERO_PAGEX); break;
		case 0x0D: ORA(AdrMode::ABSOLUTE); break;
		case 0x1D: ORA(AdrMode::ABSOLUTEX); break;
		case 0x19: ORA(AdrMode::ABSOLUTEY); break;
		case 0x01: ORA(AdrMode::INDEXED_INDIRECT); break;
		case 0x11: ORA(AdrMode::INDIRECT_INDEXED); break;
		case 0x2A: ROL(AdrMode::ACCUMULATOR); break;
		case 0x26: ROL(AdrMode::ZERO_PAGE); break;
		case 0x36: ROL(AdrMode::ZERO_PAGEX); break;
		case 0x2E: ROL(AdrMode::ABSOLUTE); break;
		case 0x3E: ROL(AdrMode::ABSOLUTEX); break;
		case 0x6A: ROR(AdrMode::ACCUMULATOR); break;
		case 0x66: ROR(AdrMode::ZERO_PAGE); break;
		case 0x76: ROR(AdrMode::ZERO_PAGEX); break;
		case 0x6E: ROR(AdrMode::ABSOLUTE); break;
		case 0x7E: ROR(AdrMode::ABSOLUTEX); break;
		case 0xE9: SBC(AdrMode::IMMEDIATE); break;
		case 0xE5: SBC(AdrMode::ZERO_PAGE); break;
		case 0xF5: SBC(AdrMode::ZERO_PAGEX); break;
		case 0xED: SBC(AdrMode::ABSOLUTE); break;
		case 0xFD: SBC(AdrMode::ABSOLUTEX); break;
		case 0xF9: SBC(AdrMode::ABSOLUTEY); break;
		case 0xE1: SBC(AdrMode::INDEXED_INDIRECT); break;
		case 0xF1: SBC(AdrMode::INDIRECT_INDEXED); break;
		case 0x85: STA(AdrMode::ZERO_PAGE); break;
		case 0x95: STA(AdrMode::ZERO_PAGEX); break;
		case 0x8D: STA(AdrMode::ABSOLUTE); break;
		case 0x9D: STA(AdrMode::ABSOLUTEX); break;
		case 0x99: STA(AdrMode::ABSOLUTEY); break;
		case 0x81: STA(AdrMode::INDEXED_INDIRECT); break;
		case 0x91: STA(AdrMode::INDIRECT_INDEXED); break;
		case 0x86: STX(AdrMode::ZERO_PAGE); break;
		case 0x96: STX(AdrMode::ZERO_PAGEY); break;
		case 0x8E: STX(AdrMode::ABSOLUTE); break;
		case 0x84: STY(AdrMode::ZERO_PAGE); break;
		case 0x94: STY(AdrMode::ZERO_PAGEY); break;
		case 0x8C: STY(AdrMode::ABSOLUTE); break;
		default:
			//throw("Invalid Instruction!");
			PC++;
			break;
		}
		

		// bit of a hack for time being relinquish control, add 6 or so cycles (yet to add cycle code)
		m_cycleCounter += 3;
	}

	void CPU::LoadCartridge(Cartridge* cartridge)
	{
		// Does not support dynamic bank loading at present, games with more than 2 banks will not work

		m_loadedCartridge = cartridge;

		const std::vector<ubyte> bank0 = cartridge->GetPRGBank(0);
		const std::vector<ubyte> bank1 = cartridge->GetPRGBank(1);

		for (int i = 0; i < bank0.size(); ++i)
		{
			RAM[PRGROM_LOWER + i] = bank0[i];
			RAM[PRGROM_UPPER + i] = bank1[i];
		}

		// load pattern tables into PPU
		const std::vector<ubyte> vrambank0 = cartridge->GetCHRBank(0);
		//const std::vector<ubyte> vrambank1 = cartridge->GetCHRBank(1);

		for (int i = 0; i < vrambank0.size(); ++i)
		{
			m_ppu->m_vram[i] = vrambank0[i];
		}

		//for (int i = 0; i < vrambank1.size(); ++i)
		//{
		//	m_ppu->m_vram[0x1FFF + i] = vrambank1[i];
		//}

		// Set the program counter to the reset vector 
		PC = ReadMemory16(0xFFFC);
		//PC = 0xC000;
	}

	void CPU::SetProcessorFlag(PFlags Flag, bool bEnabled)
	{
		if (bEnabled)
		{
			ProcessorStatus |= (uint8)Flag;
		}
		else
		{
			ProcessorStatus &= ~((uint8)Flag);
		}
	}

	/** CPU MEMORY READ & WRITE **/
	void CPU::WriteMemory8(uint16 Addr, uint8 Data)
	{
		//if (m_cycleCounter < 29658)
		//{
		//	if (Addr == PPU_DATA_ADR || Addr == PPU_MASK_ADR || Addr == PPU_ADR || Addr == PPU_SCROLL_ADR)
		//	{
		//		return;
		//	}
		//}

		RAM[Addr] = Data;

		// Write lsb of data previous written into PPU registers into ppu status $2002 register
		if ((Addr >= PPU_CTRL_ADR && Addr <= PPU_DATA_ADR) || Addr == OAM_DMA_ADR)
		{
			// Write lsb (5 bits) into PPUSTATUS register $2002
			RAM[PPU_STATUS_ADR] = (RAM[PPU_STATUS_ADR] & 0xE0) | (Data & 0x1F);
		}

		// Write to PPUDATA $2700 - VRAM Address incremented by bit 2 of $2000 (cpu ctrl address) after read/ write
		if (Addr == PPU_DATA_ADR || Addr == PPU_SCROLL_ADR)
		{
			m_ppu->WriteMemory8(m_vramAddress, Data);

			uint8 incrememnt = (this->RAM[PPU_CTRL_ADR] & (uint8)PPUCtrl::VRamAddressIncrement);
			if (incrememnt)
			{
				// Plus 32 to address 
				m_vramAddress += 32;
			}
			else
			{
				// Plus 1 to address
				m_vramAddress++;
			}
		}

		// Write OAM Address 
		if (Addr == OAM_ADR)
		{
			m_oamAddress = Data;
		}

		// Writes to OAM data $2004, increment OAMADDR after the write, reads during vertial or forced blanking return the value but don't increment
		if (Addr == OAM_DATA_ADR)
		{
			m_ppu->WriteOAMByte(m_oamAddress, Data);
			m_oamAddress++;
		}

		// OAM DMA $4014 - Initialise DMA
		if (Addr == OAM_DMA_ADR)
		{
			uint16 start = 0x100 * Data;
			for (uint offset = 0; offset <= 0xFF; ++offset)
			{
				m_ppu->WriteOAMByte(offset, ReadMemory8(start + offset));
			}

			m_cycleCounter += 513;
			m_cycleCounter += m_cycleCounter % 2;
			m_oamAddress = 0xFF;
		}

		// $2006/ $2005
		if (Addr == PPU_ADR || Addr == PPU_SCROLL_ADR)
		{
			if (!m_vramToggle)
			{
				m_vramAddress = (uint16)Data << 8;
			}
			else
			{
				m_vramAddress |= Data;
			}

			m_vramToggle = !m_vramToggle;
		}

		//!< * Memory at $000-$07FF mirrored at $0800, $1000, $1800
		//!< * Assuming any data written to range $0800-$2000 should
		//!< * Mirroring will occur appriopiately 
		if (Addr < 0x2000) {
			if (Addr < 0x0800)
			{
				//!< Mirroring at $0800, $1000 and $1800 
				this->RAM[Addr + 0x0800] = Data;
				this->RAM[Addr + 0x1000] = Data;
				this->RAM[Addr + 0x1800] = Data;
				return;
			}
			else if (Addr < 0x1000)
			{
				this->RAM[Addr - 0x0800] = Data;
				this->RAM[Addr + 0x0800] = Data;
				this->RAM[Addr + 0x1000] = Data;
				return;
			}
			else if (Addr < 0x1800)
			{
				this->RAM[Addr - 0x1000] = Data;
				this->RAM[Addr - 0x0800] = Data;
				this->RAM[Addr + 0x0800] = Data;
				return;
			}
		}

		//!< Writes to I/O register range $2000-$2007 are mirrored 
		//!< every 8 bytes in the range $2008-$3FFF

		//!< Initially check if within range or I/O regs and mirrors 
		if (Addr < 0x4000 & Addr >= 0x2000)
		{
			//!< Minus the starting position and get the remainder of the addr / 8 in order to establish 
			//!< the bytes that should be mirrored (mByte -> byte to be mirrored)
			uint16 _mByte = (Addr - 0x2000) % 8;

			//!< For every byte in range $2000 - $3FFF (kinda) 
			for (uint16 _m = _mByte; _m < 0x4000; _m += 8) {
				this->RAM[0x2000 + _m] = Data;
			}
		}
	}

	uint8 CPU::ReadMemory8(uint16 Addr)
	{
		return this->RAM[Addr];
	}

	uint16 CPU::ReadMemory16(uint16 Addr)
	{
		uint16 mem = ReadMemory8(Addr);
		mem |= (ReadMemory8(Addr + 1) << 8);
		return mem;
	}

	void CPU::PushStack8(uint8 memory)
	{
		WriteMemory8(STACK + SP, memory);
		DecrementSP();
	}

	void CPU::PushStack16(uint16 memory)
	{
		// Push high, low 
		PushStack8(memory >> 8);
		PushStack8(memory);
	}

	uint8 CPU::PopStack8()
	{
		IncrementSP();
		return ReadMemory8(STACK + SP);
	}

	uint16 CPU::PopStack16()
	{
		// pop low - high
		uint16 low = PopStack8();
		uint16 high = PopStack8() << 8;
		return high | low;
	}

	/** ======================= **/

#pragma region _6502_Instructions 

#pragma region Implied Addressing Mode Instructions
/***Implied Addressing Mode Instructions (25)***/
	void CPU::BRK_$00()
	{
		PushStack16(PC);
		PushStack8(ProcessorStatus);
		SetProcessorFlag(PFlags::BRK_CMD, true);
		PC = ReadMemory16(0xFFFE);
	}
	void CPU::CLC_$18()
	{
		SetProcessorFlag(PFlags::CARRY, false);
		PC++;
	}
	void CPU::CLD_$D8()
	{
		SetProcessorFlag(PFlags::DECIMAL_MODE, false);
		PC++;
	}
	void CPU::CLI_$58()
	{
		SetProcessorFlag(PFlags::INTERRUPT_DISABLED, false);
		PC++;
	}
	void CPU::CLV_$B8()
	{
		SetProcessorFlag(PFlags::OVER_FLOW, false);
		PC++;
	}
	void CPU::DEX_$CA()
	{
		XReg--;
		SetProcessorFlag(PFlags::ZERO, XReg == 0);
		SetProcessorFlag(PFlags::NEGATIVE, XReg & PFlags::NEGATIVE);
		PC++;
	}
	void CPU::DEY_$88()
	{
		YReg--;
		SetProcessorFlag(PFlags::ZERO, YReg == 0);
		SetProcessorFlag(PFlags::NEGATIVE, YReg & PFlags::NEGATIVE);
		PC++;
	}
	void CPU::INX_$E8()
	{
		XReg++;
		SetProcessorFlag(PFlags::ZERO, XReg == 0);
		SetProcessorFlag(PFlags::NEGATIVE, XReg & PFlags::NEGATIVE);
		PC++;
	}
	void CPU::INY_$C8()
	{
		YReg++;
		SetProcessorFlag(PFlags::ZERO, YReg == 0);
		SetProcessorFlag(PFlags::NEGATIVE, YReg & PFlags::NEGATIVE);
		PC++;
	}
	void CPU::NOP_$EA()
	{
		//!< Does nothing 
		PC++;
	}
	void CPU::PHA_$48()
	{
		PushStack8(Accumulator);
		PC++;
	}
	void CPU::PHP_$08()
	{
		PushStack8(ProcessorStatus);
		PC++;
	}
	void CPU::PLA_$68()
	{
		Accumulator = PopStack8();
		SetProcessorFlag(PFlags::ZERO, Accumulator == 0);
		SetProcessorFlag(PFlags::NEGATIVE, Accumulator & PFlags::NEGATIVE);
		PC++;
	}
	void CPU::PLP_$28()
	{
		ProcessorStatus = PopStack8();
		PC++;
	}
	void CPU::RTI_$40()
	{
		ProcessorStatus = PopStack8();
		PC = PopStack16();
	}
	void CPU::JSR_$20()
	{
		PC++;
		uint16 jmpAdr = ReadMemory16(PC);
		PushStack16(PC + 1);
		PC = jmpAdr;
	}
	void CPU::RTS_$60()
	{
		this->PC = PopStack16() + 1;
	}
	void CPU::SEC_$38()
	{
		this->ProcessorStatus |= PFlags::CARRY;
		PC++;
	}
	void CPU::SED_$f8() {
		this->ProcessorStatus |= PFlags::DECIMAL_MODE;
		PC++;
	}
	void CPU::SEI_$78() {
		this->ProcessorStatus |= PFlags::INTERRUPT_DISABLED;
		PC++;
	}
	void CPU::TAX_$AA()
	{
		XReg = Accumulator;
		SetProcessorFlag(PFlags::ZERO, XReg == 0);
		SetProcessorFlag(PFlags::NEGATIVE, XReg & PFlags::NEGATIVE);
		PC++;
	}
	void CPU::TAY_$A8() {
		YReg = Accumulator;
		SetProcessorFlag(PFlags::ZERO, YReg == 0);
		SetProcessorFlag(PFlags::NEGATIVE, YReg & PFlags::NEGATIVE);
		PC++;
	}
	void CPU::TSX_$BA() {
		this->XReg = this->SP;
		SetProcessorFlag(PFlags::ZERO, XReg == 0);
		SetProcessorFlag(PFlags::NEGATIVE, XReg & PFlags::NEGATIVE);
		PC++;
	}
	void CPU::TXA_$8A() {
		this->Accumulator = this->XReg;
		SetProcessorFlag(PFlags::ZERO, Accumulator == 0);
		SetProcessorFlag(PFlags::NEGATIVE, Accumulator & PFlags::NEGATIVE);
		PC++;
	}
	void CPU::TXS_$9A()
	{
		this->SP = this->XReg;
		PC++;
	}
	void CPU::TYA_$98() {
		this->Accumulator = this->YReg;
		SetProcessorFlag(PFlags::ZERO, Accumulator == 0);
		SetProcessorFlag(PFlags::NEGATIVE, Accumulator & PFlags::NEGATIVE);
		PC++;
	}
#pragma endregion Implied Mode

#pragma region Add memory to accumulator with carry
	/*Add memory to Accumulator with carry*/
	void CPU::ADC(AdrMode Mode)
	{
		uint8 data = ReadMemory8(ReadMemoryAddress(Mode));

		uint16 sum = ProcessorStatus & PFlags::CARRY;
		sum += Accumulator;
		sum += data;

		// check for overflow. 
		if (data & PFlags::NEGATIVE && Accumulator & PFlags::NEGATIVE)
		{
			SetProcessorFlag(PFlags::OVER_FLOW, (sum & PFlags::NEGATIVE) == 0);
		}

		// check for overflow.
		if ((data & PFlags::NEGATIVE) == 0 && (Accumulator & PFlags::NEGATIVE) == 0)
		{
			SetProcessorFlag(PFlags::OVER_FLOW, sum & PFlags::NEGATIVE);
		}

		SetProcessorFlag(PFlags::CARRY, sum > 255);
		SetProcessorFlag(PFlags::ZERO, sum == 0);
		SetProcessorFlag(PFlags::NEGATIVE, sum & PFlags::NEGATIVE);

		Accumulator = sum;
	}

#pragma endregion ADC - Add with carry

#pragma region Bitwise AND with acumulator
	/*AND - Bitwsie AND */
	void CPU::AND(AdrMode Mode)
	{
		uint8 data = ReadMemory8(ReadMemoryAddress(Mode));
		Accumulator &= data;

		SetProcessorFlag(PFlags::ZERO, Accumulator == 0);
		SetProcessorFlag(PFlags::NEGATIVE, (Accumulator & PFlags::NEGATIVE));
	}
#pragma endregion

	/*ASL - Arithmetic Shift Left*/
	void CPU::ASL(AdrMode Mode)
	{ 
		uint8 data = 0;

		if (Mode == AdrMode::ACCUMULATOR)
		{
			PC++;
			SetProcessorFlag(PFlags::CARRY, (Accumulator & PFlags::NEGATIVE));
			Accumulator = Accumulator << 1;
			data = Accumulator;
		}
		else
		{
			uint16 adr = ReadMemoryAddress(Mode);
			data = ReadMemory8(adr);
			SetProcessorFlag(PFlags::CARRY, (data & PFlags::NEGATIVE));
			data = data << 1;
			WriteMemory8(adr, data);
		}

		SetProcessorFlag(PFlags::ZERO, data == 0);
		SetProcessorFlag(PFlags::NEGATIVE, (data & PFlags::NEGATIVE));
	}

	/*BIT - Check if bit set in memory location*/
	void CPU::BIT(AdrMode Mode)
	{
		uint16 adr = ReadMemoryAddress(Mode);
		uint8 data = ReadMemory8(adr);

		SetProcessorFlag(PFlags::ZERO, (data & Accumulator) == 0);
		SetProcessorFlag(PFlags::OVER_FLOW, data & PFlags::OVER_FLOW);
		SetProcessorFlag(PFlags::NEGATIVE, data & PFlags::NEGATIVE);
	}

	void CPU::CMP(AdrMode Mode)
	{
		// Used to determine if value in memory is less than, greater than or equal to value in accumulator
		uint8 memory = ReadMemory8(ReadMemoryAddress(Mode));
		SetProcessorFlag(PFlags::NEGATIVE, (Accumulator - memory) & PFlags::NEGATIVE);
		SetProcessorFlag(PFlags::CARRY, Accumulator >= memory);
		SetProcessorFlag(PFlags::ZERO, Accumulator == memory);
	}

	void CPU::CPX(AdrMode Mode)
	{
		uint8 memory = ReadMemory8(ReadMemoryAddress(Mode));
		SetProcessorFlag(PFlags::NEGATIVE, (XReg - memory) & PFlags::NEGATIVE);
		SetProcessorFlag(PFlags::CARRY, XReg >= memory);
		SetProcessorFlag(PFlags::ZERO, XReg == memory);
	}

	void CPU::CPY(AdrMode Mode)
	{
		uint8 memory = ReadMemory8(ReadMemoryAddress(Mode));
		SetProcessorFlag(PFlags::NEGATIVE, (YReg - memory) & PFlags::NEGATIVE);
		SetProcessorFlag(PFlags::CARRY, YReg >= memory);
		SetProcessorFlag(PFlags::ZERO, YReg == memory);
	}

	void CPU::DEC(AdrMode Mode)
	{
		uint16 adr = ReadMemoryAddress(Mode);
		uint8 data = ReadMemory8(adr);
		data--;
		WriteMemory8(adr, data);
		SetProcessorFlag(PFlags::ZERO, data == 0);
		SetProcessorFlag(PFlags::NEGATIVE, data & PFlags::NEGATIVE);
	}

	void CPU::EOR(AdrMode Mode)
	{
		uint8 memory = ReadMemory8(ReadMemoryAddress(Mode));
		Accumulator ^= memory;
		SetProcessorFlag(PFlags::ZERO, Accumulator == 0);
		SetProcessorFlag(PFlags::NEGATIVE, Accumulator & PFlags::NEGATIVE);
	}

	void CPU::INC(AdrMode Mode)
	{
		uint16 adr = ReadMemoryAddress(Mode);
		uint8 data = ReadMemory8(adr);
		data++;
		WriteMemory8(adr, data);
		SetProcessorFlag(PFlags::ZERO, data == 0);
		SetProcessorFlag(PFlags::NEGATIVE, data & PFlags::NEGATIVE);
	}

	void CPU::JMP(AdrMode Mode)
	{
		PC = ReadMemoryAddress(Mode);
	}

	void CPU::ORA(AdrMode Mode)
	{
		uint8 memory = ReadMemory8(ReadMemoryAddress(Mode));
		Accumulator |= memory;
		SetProcessorFlag(PFlags::ZERO, Accumulator == 0);
		SetProcessorFlag(PFlags::NEGATIVE, Accumulator & PFlags::NEGATIVE);
	}

	void CPU::ROL(AdrMode Mode)
	{
		uint8 data = 0;
		uint8 carry = (ProcessorStatus & PFlags::CARRY) ? 0x01 : 0;

		if (Mode == AdrMode::ACCUMULATOR)
		{
			PC++;
			SetProcessorFlag(PFlags::CARRY, (Accumulator & PFlags::NEGATIVE));
			Accumulator = Accumulator << 1;
			Accumulator |= carry;
			data = Accumulator;
		}
		else
		{
			uint16 adr = ReadMemoryAddress(Mode);
			data = ReadMemory8(adr);
			SetProcessorFlag(PFlags::CARRY, (data & PFlags::NEGATIVE));
			data = data << 1;
			Accumulator |= carry;
			WriteMemory8(adr, data);
		}

		SetProcessorFlag(PFlags::ZERO, data == 0);
		SetProcessorFlag(PFlags::NEGATIVE, (data & PFlags::NEGATIVE));
	}

	void CPU::ROR(AdrMode Mode)
	{
		uint8 data = 0;
		uint8 carry = (ProcessorStatus & PFlags::CARRY) ? 0x80 : 0;

		if (Mode == AdrMode::ACCUMULATOR)
		{
			PC++;
			SetProcessorFlag(PFlags::CARRY, (Accumulator & PFlags::CARRY));
			Accumulator = Accumulator >> 1;
			Accumulator |= carry;
			data = Accumulator;
		}
		else
		{
			uint16 adr = ReadMemoryAddress(Mode);
			data = ReadMemory8(adr);
			SetProcessorFlag(PFlags::CARRY, (data & PFlags::CARRY));
			data = data >> 1;
			data |= carry;
			WriteMemory8(adr, data);
		}

		SetProcessorFlag(PFlags::ZERO, data == 0);
		SetProcessorFlag(PFlags::NEGATIVE, (data & PFlags::NEGATIVE));
	}

	void CPU::SBC(AdrMode Mode)
	{
		uint8 memory = ReadMemory8(ReadMemoryAddress(Mode));

		uint16 sum = Accumulator;
		sum -= memory;
		sum -= (1 - (ProcessorStatus & PFlags::CARRY));

		bool overflow = false; 

		if ((Accumulator ^ sum) & (memory ^ sum) & PFlags::NEGATIVE)
		{
			SetProcessorFlag(PFlags::OVER_FLOW, true);
			SetProcessorFlag(PFlags::CARRY, false);
		}
		else
		{
			SetProcessorFlag(PFlags::OVER_FLOW, false);
		}

		SetProcessorFlag(PFlags::ZERO, sum == 0);
		SetProcessorFlag(PFlags::NEGATIVE, (sum & PFlags::NEGATIVE));
		Accumulator = sum;
	}

	void CPU::STA(AdrMode Mode)
	{
		WriteMemory8(ReadMemoryAddress(Mode), Accumulator);
	}

	void CPU::STX(AdrMode Mode)
	{
		WriteMemory8(ReadMemoryAddress(Mode), XReg);
	}

	void CPU::STY(AdrMode Mode)
	{
		WriteMemory8(ReadMemoryAddress(Mode), YReg);
	}

	void CPU::BCS_$B0(AdrMode Mode)
	{
		uint16 memory = ReadMemoryAddress(Mode);

		if (ProcessorStatus & PFlags::CARRY)
		{
			PC = memory;
		}
	}

	void CPU::BEQ_$F0(AdrMode Mode)
	{
		uint16 memory = ReadMemoryAddress(Mode);

		if (ProcessorStatus & PFlags::ZERO)
		{
			PC = memory;
		}
	}

	void CPU::BMI_$30(AdrMode Mode)
	{
		uint16 memory = ReadMemoryAddress(Mode);

		if (ProcessorStatus & PFlags::NEGATIVE)
		{
			PC = memory;
		}
	}

	void CPU::BNE_D0(AdrMode Mode)
	{
		uint16 memory = ReadMemoryAddress(Mode);

		if ((ProcessorStatus & PFlags::ZERO) == 0)
		{
			PC = memory;
		}
	}

	void CPU::BPL_$10(AdrMode Mode)
	{
		uint16 memory = ReadMemoryAddress(Mode);

		if ((ProcessorStatus & PFlags::NEGATIVE) == 0)
		{
			PC = memory;
		}
	}

	void CPU::BVC_$50(AdrMode Mode)
	{
		uint16 memory = ReadMemoryAddress(Mode);

		if ((ProcessorStatus & PFlags::OVER_FLOW) == 0)
		{
			PC = memory;
		}
	}

	void CPU::BVS_$70(AdrMode Mode)
	{
		uint16 memory = ReadMemoryAddress(Mode);

		if (ProcessorStatus & PFlags::OVER_FLOW)
		{
			PC = memory;
		}
	}

	void CPU::BCC_$90(AdrMode Mode)
	{
		uint16 memory = ReadMemoryAddress(Mode);

		if ((ProcessorStatus & PFlags::CARRY) == 0)
		{
			PC = memory;
		}
	}

	void CPU::LDA(AdrMode Mode)
	{
		uint8 memory = ReadMemory8(ReadMemoryAddress(Mode));
		Accumulator = memory;
		SetProcessorFlag(PFlags::ZERO, Accumulator == 0);
		SetProcessorFlag(PFlags::NEGATIVE, (Accumulator & PFlags::NEGATIVE));
	}

	void CPU::LDX(AdrMode Mode)
	{
		uint8 memory = ReadMemory8(ReadMemoryAddress(Mode));
		XReg = memory;
		SetProcessorFlag(PFlags::ZERO, XReg == 0);
		SetProcessorFlag(PFlags::NEGATIVE, (XReg & PFlags::NEGATIVE));
	}

	void CPU::LDY(AdrMode Mode)
	{
		uint8 memory = ReadMemory8(ReadMemoryAddress(Mode));
		YReg = memory;
		SetProcessorFlag(PFlags::ZERO, YReg == 0);
		SetProcessorFlag(PFlags::NEGATIVE, (YReg & PFlags::NEGATIVE));
	}

	void CPU::LSR(AdrMode Mode)
	{
		uint8 data = 0;

		if (Mode == AdrMode::ACCUMULATOR)
		{
			PC++;
			SetProcessorFlag(PFlags::CARRY, (Accumulator & PFlags::CARRY));
			Accumulator = Accumulator >> 1;
			data = Accumulator;
		}
		else
		{
			uint16 adr = ReadMemoryAddress(Mode);
			data = ReadMemory8(adr);
			SetProcessorFlag(PFlags::CARRY, (data & PFlags::CARRY));
			data = data >> 1;
			WriteMemory8(adr, data);
		}

		SetProcessorFlag(PFlags::ZERO, data == 0);
		SetProcessorFlag(PFlags::NEGATIVE, (data & PFlags::NEGATIVE));
	}

#pragma endregion instructions/ operation codes used by the 6502 

	// Helper Functions 
	void CPU::IncrementSP() {
		(this->SP == 0xFF) ? this->SP = 0 : this->SP++;
	}

	void CPU::DecrementSP() {
		(this->SP == 0) ? this->SP = 0xFF : this->SP--;
	}

	uint16 CPU::ReadMemoryAddress(AdrMode Mode)
	{
		switch (Mode)
		{
		case AdrMode::ZERO_PAGE: return GetMemZeroPage();
		case AdrMode::ZERO_PAGEX: return GetMemZeroPageX();
		case AdrMode::ZERO_PAGEY: return GetMemZeroPageY();
		case AdrMode::ABSOLUTE: return GetMemAbsolute();
		case AdrMode::ABSOLUTEX: return GetMemAbsoluteX();
		case AdrMode::ABSOLUTEY: return GetMemAbsoluteY();
		case AdrMode::ABSOLUTE_INDIRECT: return GetMemAbsoluteIndirect();
		case AdrMode::IMMEDIATE: return GetMemImmediate();
		case AdrMode::RELATIVE: return GetMemRelative();
		case AdrMode::INDEXED_INDIRECT: return GetMemIndexedIndirect();
		case AdrMode::INDIRECT_INDEXED: return GetMemIndirectIndexed();
		default:
			throw ("Operation Addressing Mode Error");
		}

		return uint8();
	}

	uint16 CPU::GetMemZeroPage()
	{
		PC++;
		uint8 M = ReadMemory8(PC);
		PC++;
		return M;
	}

	uint16 CPU::GetMemZeroPageX()
	{
		PC++;
		uint8 M = (uint8)(ReadMemory8(PC) + XReg);
		PC++;
		return M;
	}

	uint16 CPU::GetMemZeroPageY()
	{
		PC++;
		uint8 M = (uint8)(ReadMemory8(PC) + YReg);
		PC++;
		return M;
	}

	uint16 CPU::GetMemAbsolute()
	{
		PC++;
		uint16 Adr = ReadMemory16(PC);
		PC += 2;
		return Adr;
	}

	uint16 CPU::GetMemAbsoluteX()
	{
		PC++;
		uint16 Adr = ReadMemory16(PC);
		uint16 M = Adr + XReg;
		PC += 2;
		return M;
	}

	uint16 CPU::GetMemAbsoluteY()
	{
		PC++;
		uint16 Adr = ReadMemory16(PC);
		uint16 M = Adr + YReg;
		PC += 2;
		return M;
	}

	uint16 CPU::GetMemAbsoluteIndirect()
	{
		PC++;
		uint16 Adr;

		// http://www.obelisk.me.uk/6502/reference.html#JMP
		// An original 6502 has does not correctly fetch the target address if the indirect vector falls on a page boundary (e.g. $xxFF where xx is any value from $00 to $FF). In this case fetches the LSB from $xxFF as expected but takes the MSB from $xx00. This is fixed in some later chips like the 65SC02 so for compatibility always ensure the indirect vector is not at the end of the page.
		uint8 lsb = ReadMemory8(PC);

		if (lsb == 0xFF)
		{
			uint16 msb = ReadMemory8(PC+1);

			Adr = ReadMemory8(ReadMemory16(PC));
			Adr |= (((uint16)ReadMemory8(msb << 8)) << 8);
		}
		else
		{
			Adr = ReadMemory16(ReadMemory16(PC));
		}

		PC += 2;
		return Adr;
	}

	uint16 CPU::GetMemImmediate()
	{
		PC++;
		uint16 M = PC;
		PC++;
		return M;
	}

	uint16 CPU::GetMemRelative()
	{
		PC+=2;
		uint16 M = PC + (int8)ReadMemory8(PC-1);
		return M;
	}

	uint16 CPU::GetMemIndexedIndirect()
	{
		PC++;
		uint16 Adr = ReadMemory16(ReadMemory8(PC) + XReg);
		PC++;
		return Adr;
	}

	uint16 CPU::GetMemIndirectIndexed()
	{
		PC++;
		uint16 Adr = ReadMemory16(ReadMemory8(PC));
		uint16 M = Adr + YReg;
		PC++;
		return M;
	}
}