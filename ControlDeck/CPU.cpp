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

	void CPU::Init()
	{
		InitInstructions();
	}

	void CPU::AddInstruction(SharedPtr<Instruction> instruction)
	{
		std::vector<uint8> opcodes = instruction->GetOpCodes();

		for (uint8 opcode : opcodes)
		{
			m_instructions[opcode] = instruction;
			m_instructions[opcode]->m_cpu = this;
		}
	}

	void CPU::InitInstructions()
	{
		m_instructions.resize(0xFF);

		SharedPtr<Instruction> BRK = std::make_shared<Instruction>("BRK", &CPU::BRK_$00, this);
		BRK->AddOperation(0x00, 1, 7, 0, AdrMode::IMPLIED);
		AddInstruction(BRK);

		SharedPtr<Instruction> ADC = std::make_shared<Instruction>("ADC", &CPU::ADC, this);
		ADC->AddOperation(0x69, 2, 2, 0, AdrMode::IMMEDIATE);
		ADC->AddOperation(0x65, 2, 3, 0, AdrMode::ZERO_PAGE);
		ADC->AddOperation(0x75, 2, 4, 0, AdrMode::ZERO_PAGEX);
		ADC->AddOperation(0x6D, 3, 4, 0, AdrMode::ABSOLUTE);
		ADC->AddOperation(0x7D, 3, 4, 1, AdrMode::ABSOLUTEX);
		ADC->AddOperation(0x79, 3, 4, 1, AdrMode::ABSOLUTEY);
		ADC->AddOperation(0x61, 2, 6, 0, AdrMode::INDEXED_INDIRECT);
		ADC->AddOperation(0x71, 2, 5, 1, AdrMode::INDIRECT_INDEXED);
		AddInstruction(ADC);

		SharedPtr<Instruction> AND = std::make_shared<Instruction>("AND", &CPU::AND, this);
		AND->AddOperation(0x29, 2, 2, 0, AdrMode::IMMEDIATE);
		AND->AddOperation(0x25, 2, 3, 0, AdrMode::ZERO_PAGE);
		AND->AddOperation(0x35, 2, 4, 0, AdrMode::ZERO_PAGEX);
		AND->AddOperation(0x2D, 3, 4, 0, AdrMode::ABSOLUTE);
		AND->AddOperation(0x3D, 3, 4, 1, AdrMode::ABSOLUTEX);
		AND->AddOperation(0x39, 3, 4, 1, AdrMode::ABSOLUTEY);
		AND->AddOperation(0x21, 2, 6, 0, AdrMode::INDEXED_INDIRECT);
		AND->AddOperation(0x31, 2, 5, 1, AdrMode::INDIRECT_INDEXED);
		AddInstruction(AND);

		SharedPtr<Instruction> ASL = std::make_shared<Instruction>("ASL", &CPU::ASL, this);
		ASL->AddOperation(0x0A, 1, 2, 0, AdrMode::ACCUMULATOR);
		ASL->AddOperation(0x06, 2, 5, 0, AdrMode::ZERO_PAGE);
		ASL->AddOperation(0x16, 2, 6, 0, AdrMode::ZERO_PAGEX);
		ASL->AddOperation(0x0E, 3, 6, 0, AdrMode::ABSOLUTE);
		ASL->AddOperation(0x1E, 3, 7, 0, AdrMode::ABSOLUTEX);
		AddInstruction(ASL);

		SharedPtr<Instruction> BCC = std::make_shared<Instruction>("BCC", &CPU::BCC_$90, this);
		BCC->AddOperation(0x90, 2, 2, 0, AdrMode::RELATIVE);
		AddInstruction(BCC);

		SharedPtr<Instruction> BCS = std::make_shared<Instruction>("BCS", &CPU::BCS_$B0, this);
		BCS->AddOperation(0xB0, 2, 2, 0, AdrMode::RELATIVE);
		AddInstruction(BCS);

		SharedPtr<Instruction> BEQ = std::make_shared<Instruction>("BEQ", &CPU::BEQ_$F0, this);
		BEQ->AddOperation(0xF0, 2, 2, 0, AdrMode::RELATIVE);
		AddInstruction(BEQ);

		SharedPtr<Instruction> BIT = std::make_shared<Instruction>("BIT", &CPU::BIT, this);
		BIT->AddOperation(0x24, 2, 3, 0, AdrMode::ZERO_PAGE);
		BIT->AddOperation(0x2C, 3, 3, 0, AdrMode::ABSOLUTE);
		AddInstruction(BIT);

		SharedPtr<Instruction> BMI = std::make_shared<Instruction>("BMI", &CPU::BMI_$30, this);
		BMI->AddOperation(0x30, 2, 2, 0, AdrMode::RELATIVE);
		AddInstruction(BMI);

		SharedPtr<Instruction> BNE = std::make_shared<Instruction>("BNE", &CPU::BNE_D0, this);
		BNE->AddOperation(0xD0, 2, 2, 0, AdrMode::RELATIVE);
		AddInstruction(BNE);

		SharedPtr<Instruction> BPL = std::make_shared<Instruction>("BPL", &CPU::BPL_$10, this);
		BPL->AddOperation(0x10, 2, 2, 0, AdrMode::RELATIVE);
		AddInstruction(BPL);

		SharedPtr<Instruction> BVC = std::make_shared<Instruction>("BVC", &CPU::BVC_$50, this);
		BVC->AddOperation(0x50, 2, 2, 0, AdrMode::RELATIVE);
		AddInstruction(BVC);

		SharedPtr<Instruction> BVS = std::make_shared<Instruction>("BVS", &CPU::BVS_$70, this);
		BVS->AddOperation(0x70, 2, 2, 0, AdrMode::RELATIVE);
		AddInstruction(BVS);

		SharedPtr<Instruction> CLC = std::make_shared<Instruction>("CLC", &CPU::CLC_$18, this);
		CLC->AddOperation(0x18, 1, 2, 0, AdrMode::IMPLIED);
		AddInstruction(CLC);

		SharedPtr<Instruction> CLD = std::make_shared<Instruction>("CLD", &CPU::CLD_$D8, this);
		CLD->AddOperation(0xD8, 1, 2, 0, AdrMode::IMPLIED);
		AddInstruction(CLD);

		SharedPtr<Instruction> CLI = std::make_shared<Instruction>("CLI", &CPU::CLI_$58, this);
		CLI->AddOperation(0x58, 1, 2, 0, AdrMode::IMPLIED);
		AddInstruction(CLI);

		SharedPtr<Instruction> CLV = std::make_shared<Instruction>("CLV", &CPU::CLV_$B8, this);
		CLV->AddOperation(0xB8, 1, 2, 0, AdrMode::IMPLIED);
		AddInstruction(CLV);

		SharedPtr<Instruction> CMP = std::make_shared<Instruction>("CMP", &CPU::CMP, this);
		CMP->AddOperation(0xC9, 2, 2, 0, AdrMode::IMMEDIATE);
		CMP->AddOperation(0xC5, 2, 3, 0, AdrMode::ZERO_PAGE);
		CMP->AddOperation(0xD5, 2, 4, 0, AdrMode::ZERO_PAGEX);
		CMP->AddOperation(0xCD, 3, 4, 0, AdrMode::ABSOLUTE);
		CMP->AddOperation(0xDD, 3, 4, 1, AdrMode::ABSOLUTEX);
		CMP->AddOperation(0xD9, 3, 4, 1, AdrMode::ABSOLUTEY);
		CMP->AddOperation(0xC1, 2, 6, 0, AdrMode::INDEXED_INDIRECT);
		CMP->AddOperation(0xD1, 2, 5, 1, AdrMode::INDIRECT_INDEXED);
		AddInstruction(CMP);

		SharedPtr<Instruction> CPX = std::make_shared<Instruction>("CPX", &CPU::CPX, this);
		CPX->AddOperation(0xE0, 2, 2, 0, AdrMode::IMMEDIATE);
		CPX->AddOperation(0xE4, 2, 3, 0, AdrMode::ZERO_PAGE);
		CPX->AddOperation(0xEC, 3, 4, 0, AdrMode::ABSOLUTE);
		AddInstruction(CPX);

		SharedPtr<Instruction> CPY = std::make_shared<Instruction>("CPY", &CPU::CPY, this);
		CPY->AddOperation(0xC0, 2, 2, 0, AdrMode::IMMEDIATE);
		CPY->AddOperation(0xC4, 2, 3, 0, AdrMode::ZERO_PAGE);
		CPY->AddOperation(0xCC, 3, 4, 0, AdrMode::ABSOLUTE);
		AddInstruction(CPY);

		SharedPtr<Instruction> DEC = std::make_shared<Instruction>("DEC", &CPU::DEC, this);
		DEC->AddOperation(0xC6, 2, 5, 0, AdrMode::ZERO_PAGE);
		DEC->AddOperation(0xD6, 2, 6, 0, AdrMode::ZERO_PAGEX);
		DEC->AddOperation(0xCE, 3, 6, 0, AdrMode::ABSOLUTE);
		DEC->AddOperation(0xDE, 3, 7, 0, AdrMode::ABSOLUTEX);
		AddInstruction(DEC);

		SharedPtr<Instruction> DEX = std::make_shared<Instruction>("DEX", &CPU::DEX_$CA, this);
		DEX->AddOperation(0xCA, 1, 2, 0, AdrMode::IMPLIED);
		AddInstruction(DEX);

		SharedPtr<Instruction> DEY = std::make_shared<Instruction>("DEY", &CPU::DEY_$88, this);
		DEY->AddOperation(0x88, 1, 2, 0, AdrMode::IMPLIED);
		AddInstruction(DEY);

		SharedPtr<Instruction> EOR = std::make_shared<Instruction>("EOR", &CPU::EOR, this);
		EOR->AddOperation(0x49, 2, 2, 0, AdrMode::IMMEDIATE);
		EOR->AddOperation(0x45, 2, 3, 0, AdrMode::ZERO_PAGE);
		EOR->AddOperation(0x55, 2, 4, 0, AdrMode::ZERO_PAGEX);
		EOR->AddOperation(0x4D, 3, 4, 0, AdrMode::ABSOLUTE);
		EOR->AddOperation(0x5D, 3, 4, 1, AdrMode::ABSOLUTEX);
		EOR->AddOperation(0x59, 3, 4, 1, AdrMode::ABSOLUTEY);
		EOR->AddOperation(0x41, 2, 6, 0, AdrMode::INDEXED_INDIRECT);
		EOR->AddOperation(0x51, 2, 5, 1, AdrMode::INDIRECT_INDEXED);
		AddInstruction(EOR);

		SharedPtr<Instruction> INC = std::make_shared<Instruction>("INC", &CPU::INC, this);
		INC->AddOperation(0xE6, 2, 2, 0, AdrMode::ZERO_PAGE);
		INC->AddOperation(0xF6, 2, 3, 0, AdrMode::ZERO_PAGE);
		INC->AddOperation(0xEE, 2, 4, 0, AdrMode::ABSOLUTE);
		INC->AddOperation(0xFE, 3, 4, 0, AdrMode::ABSOLUTEX);
		AddInstruction(INC);

		SharedPtr<Instruction> INX = std::make_shared<Instruction>("INX", &CPU::INX_$E8, this);
		INX->AddOperation(0xE8, 1, 2, 0, AdrMode::IMPLIED);
		AddInstruction(INX);

		SharedPtr<Instruction> INY = std::make_shared<Instruction>("INY", &CPU::INY_$C8, this);
		INY->AddOperation(0xC8, 1, 2, 0, AdrMode::IMPLIED);
		AddInstruction(INY);

		SharedPtr<Instruction> JMP = std::make_shared<Instruction>("JMP", &CPU::JMP, this);
		JMP->AddOperation(0x4C, 3, 3, 0, AdrMode::ABSOLUTE);
		JMP->AddOperation(0x6C, 3, 5, 0, AdrMode::ABSOLUTE_INDIRECT);
		AddInstruction(JMP);

		SharedPtr<Instruction> JSR = std::make_shared<Instruction>("JSR", &CPU::JSR_$20, this);
		JSR->AddOperation(0x20, 3, 6, 0, AdrMode::ABSOLUTE);
		AddInstruction(JSR);

		SharedPtr<Instruction> LDA = std::make_shared<Instruction>("LDA", &CPU::LDA, this);
		LDA->AddOperation(0xA9, 2, 2, 0, AdrMode::IMMEDIATE);
		LDA->AddOperation(0xA5, 2, 3, 0, AdrMode::ZERO_PAGE);
		LDA->AddOperation(0xB5, 2, 4, 0, AdrMode::ZERO_PAGEX);
		LDA->AddOperation(0xAD, 3, 4, 0, AdrMode::ABSOLUTE);
		LDA->AddOperation(0xBD, 3, 4, 1, AdrMode::ABSOLUTEX);
		LDA->AddOperation(0xB9, 3, 4, 1, AdrMode::ABSOLUTEY);
		LDA->AddOperation(0xA1, 2, 6, 0, AdrMode::INDEXED_INDIRECT);
		LDA->AddOperation(0xB1, 2, 5, 1, AdrMode::INDIRECT_INDEXED);
		AddInstruction(LDA);

		SharedPtr<Instruction> LDX = std::make_shared<Instruction>("LDX", &CPU::LDX, this);
		LDX->AddOperation(0xA2, 2, 2, 0, AdrMode::IMMEDIATE);
		LDX->AddOperation(0xA6, 2, 3, 0, AdrMode::ZERO_PAGE);
		LDX->AddOperation(0xB6, 2, 4, 0, AdrMode::ZERO_PAGEY);
		LDX->AddOperation(0xAE, 3, 4, 0, AdrMode::ABSOLUTE);
		LDX->AddOperation(0xBE, 3, 4, 1, AdrMode::ABSOLUTEY);
		AddInstruction(LDX);

		SharedPtr<Instruction> LDY = std::make_shared<Instruction>("LDY", &CPU::LDY, this);
		LDY->AddOperation(0xA0, 2, 2, 0, AdrMode::IMMEDIATE);
		LDY->AddOperation(0xA4, 2, 3, 0, AdrMode::ZERO_PAGE);
		LDY->AddOperation(0xB4, 2, 4, 0, AdrMode::ZERO_PAGEX);
		LDY->AddOperation(0xAC, 3, 4, 0, AdrMode::ABSOLUTE);
		LDY->AddOperation(0xBC, 3, 4, 1, AdrMode::ABSOLUTEX);
		AddInstruction(LDY);

		SharedPtr<Instruction> LSR = std::make_shared<Instruction>("LSR", &CPU::LSR, this);
		LSR->AddOperation(0x4A, 1, 2, 0, AdrMode::ACCUMULATOR);
		LSR->AddOperation(0x46, 2, 5, 0, AdrMode::ZERO_PAGE);
		LSR->AddOperation(0x56, 2, 6, 0, AdrMode::ZERO_PAGEX);
		LSR->AddOperation(0x4E, 3, 6, 0, AdrMode::ABSOLUTE);
		LSR->AddOperation(0x5E, 3, 7, 1, AdrMode::ABSOLUTEX);
		AddInstruction(LSR);

		SharedPtr<Instruction> NOP = std::make_shared<Instruction>("NOP", &CPU::NOP_$EA, this);
		NOP->AddOperation(0xEA, 1, 2, 0, AdrMode::IMPLIED);
		AddInstruction(NOP);

		SharedPtr<Instruction> ORA = std::make_shared<Instruction>("ORA", &CPU::ORA, this);
		ORA->AddOperation(0x09, 2, 2, 0, AdrMode::IMMEDIATE);
		ORA->AddOperation(0x05, 2, 3, 0, AdrMode::ZERO_PAGE);
		ORA->AddOperation(0x15, 2, 4, 0, AdrMode::ZERO_PAGEX);
		ORA->AddOperation(0x0D, 3, 4, 0, AdrMode::ABSOLUTE);
		ORA->AddOperation(0x1D, 3, 4, 1, AdrMode::ABSOLUTEX);
		ORA->AddOperation(0x19, 3, 4, 1, AdrMode::ABSOLUTEY);
		ORA->AddOperation(0x01, 2, 6, 0, AdrMode::INDEXED_INDIRECT);
		ORA->AddOperation(0x11, 2, 5, 1, AdrMode::INDIRECT_INDEXED);
		AddInstruction(ORA);

		SharedPtr<Instruction> PHA = std::make_shared<Instruction>("PHA", &CPU::PHA_$48, this);
		PHA->AddOperation(0x48, 1, 3, 0, AdrMode::IMPLIED);
		AddInstruction(PHA);

		SharedPtr<Instruction> PHP = std::make_shared<Instruction>("PHP", &CPU::PHP_$08, this);
		PHP->AddOperation(0x08, 1, 3, 0, AdrMode::IMPLIED);
		AddInstruction(PHP);

		SharedPtr<Instruction> PLA = std::make_shared<Instruction>("PLA", &CPU::PLA_$68, this);
		PLA->AddOperation(0x68, 1, 4, 0, AdrMode::IMPLIED);
		AddInstruction(PLA);

		SharedPtr<Instruction> PLP = std::make_shared<Instruction>("PLP", &CPU::PLP_$28, this);
		PLP->AddOperation(0x28, 1, 4, 0, AdrMode::IMPLIED);
		AddInstruction(PLP);

		SharedPtr<Instruction> ROL = std::make_shared<Instruction>("ROL", &CPU::ROL, this);
		ROL->AddOperation(0x2A, 1, 2, 0, AdrMode::ACCUMULATOR);
		ROL->AddOperation(0x26, 2, 5, 0, AdrMode::ZERO_PAGE);
		ROL->AddOperation(0x36, 2, 6, 0, AdrMode::ZERO_PAGEX);
		ROL->AddOperation(0x2E, 3, 6, 0, AdrMode::ABSOLUTE);
		ROL->AddOperation(0x3E, 3, 7, 1, AdrMode::ABSOLUTEX);
		AddInstruction(ROL);

		SharedPtr<Instruction> ROR = std::make_shared<Instruction>("ROR", &CPU::ROR, this);
		ROR->AddOperation(0x6A, 1, 2, 0, AdrMode::ACCUMULATOR);
		ROR->AddOperation(0x66, 2, 5, 0, AdrMode::ZERO_PAGE);
		ROR->AddOperation(0x76, 2, 6, 0, AdrMode::ZERO_PAGEX);
		ROR->AddOperation(0x6E, 3, 6, 0, AdrMode::ABSOLUTE);
		ROR->AddOperation(0x7E, 3, 7, 1, AdrMode::ABSOLUTEX);
		AddInstruction(ROR);

		SharedPtr<Instruction> RTI = std::make_shared<Instruction>("RTI", &CPU::RTI_$40, this);
		RTI->AddOperation(0x40, 1, 6, 0, AdrMode::IMPLIED);
		AddInstruction(RTI);

		SharedPtr<Instruction> RTS = std::make_shared<Instruction>("RTS", &CPU::RTS_$60, this);
		RTS->AddOperation(0x60, 1, 6, 0, AdrMode::IMPLIED);
		AddInstruction(RTS);

		SharedPtr<Instruction> SBC = std::make_shared<Instruction>("SBC", &CPU::SBC, this);
		SBC->AddOperation(0xE9, 2, 2, 0, AdrMode::IMMEDIATE);
		SBC->AddOperation(0xE5, 2, 3, 0, AdrMode::ZERO_PAGE);
		SBC->AddOperation(0xF5, 2, 4, 0, AdrMode::ZERO_PAGEX);
		SBC->AddOperation(0xED, 3, 4, 0, AdrMode::ABSOLUTE);
		SBC->AddOperation(0xFD, 3, 4, 1, AdrMode::ABSOLUTEX);
		SBC->AddOperation(0xF9, 3, 4, 1, AdrMode::ABSOLUTEY);
		SBC->AddOperation(0xE1, 2, 6, 0, AdrMode::INDEXED_INDIRECT);
		SBC->AddOperation(0xF1, 2, 5, 1, AdrMode::INDIRECT_INDEXED);
		AddInstruction(SBC);

		SharedPtr<Instruction> SEC = std::make_shared<Instruction>("SEC", &CPU::SEC_$38, this);
		SEC->AddOperation(0x38, 1, 2, 0, AdrMode::IMPLIED);
		AddInstruction(SEC);

		SharedPtr<Instruction> SED = std::make_shared<Instruction>("SED", &CPU::SED_$f8, this);
		SEC->AddOperation(0xF8, 1, 2, 0, AdrMode::IMPLIED);
		AddInstruction(SED);

		SharedPtr<Instruction> SEI = std::make_shared<Instruction>("SEI", &CPU::SEI_$78, this);
		SEI->AddOperation(0x78, 1, 2, 0, AdrMode::IMPLIED);
		AddInstruction(SEI);

		SharedPtr<Instruction> STA = std::make_shared<Instruction>("STA", &CPU::STA, this);
		STA->AddOperation(0x85, 2, 3, 0, AdrMode::ZERO_PAGE);
		STA->AddOperation(0x95, 2, 4, 0, AdrMode::ZERO_PAGEX);
		STA->AddOperation(0x8D, 3, 4, 0, AdrMode::ABSOLUTE);
		STA->AddOperation(0x9D, 3, 5, 0, AdrMode::ABSOLUTEX);
		STA->AddOperation(0x99, 3, 5, 0, AdrMode::ABSOLUTEY);
		STA->AddOperation(0x81, 2, 6, 0, AdrMode::INDEXED_INDIRECT);
		STA->AddOperation(0x91, 2, 6, 0, AdrMode::INDIRECT_INDEXED);
		AddInstruction(STA);

		SharedPtr<Instruction> STX = std::make_shared<Instruction>("STX", &CPU::STX, this);
		STX->AddOperation(0x86, 2, 3, 0, AdrMode::ZERO_PAGE);
		STX->AddOperation(0x96, 2, 4, 0, AdrMode::ZERO_PAGEY);
		STX->AddOperation(0x8E, 3, 4, 0, AdrMode::ABSOLUTE);
		AddInstruction(STX);

		SharedPtr<Instruction> STY = std::make_shared<Instruction>("STY", &CPU::STY, this);
		STY->AddOperation(0x84, 2, 3, 0, AdrMode::ZERO_PAGE);
		STY->AddOperation(0x94, 2, 4, 0, AdrMode::ZERO_PAGEX);
		STY->AddOperation(0x8C, 3, 4, 0, AdrMode::ABSOLUTE);
		AddInstruction(STY);

		SharedPtr<Instruction> TAX = std::make_shared<Instruction>("TAX", &CPU::TAX_$AA, this);
		TAX->AddOperation(0xAA, 1, 2, 0, AdrMode::IMPLIED);
		AddInstruction(TAX);

		SharedPtr<Instruction> TAY = std::make_shared<Instruction>("TAY", &CPU::TAY_$A8, this);
		TAY->AddOperation(0xA8, 1, 2, 0, AdrMode::IMPLIED);
		AddInstruction(TAY);

		SharedPtr<Instruction> TSX = std::make_shared<Instruction>("TSX", &CPU::TSX_$BA, this);
		TSX->AddOperation(0xBA, 1, 2, 0, AdrMode::IMPLIED);
		AddInstruction(TSX);

		SharedPtr<Instruction> TXA = std::make_shared<Instruction>("TXA", &CPU::TXA_$8A, this);
		TXA->AddOperation(0x8A, 1, 2, 0, AdrMode::IMPLIED);
		AddInstruction(TXA);

		SharedPtr<Instruction> TXS = std::make_shared<Instruction>("TXS", &CPU::TXS_$9A, this);
		TXS->AddOperation(0x9A, 1, 2, 0, AdrMode::IMPLIED);
		AddInstruction(TXS);

		SharedPtr<Instruction> TYA = std::make_shared<Instruction>("TYA", &CPU::TYA_$98, this);
		TYA->AddOperation(0x98, 1, 2, 0, AdrMode::IMPLIED);
		AddInstruction(TYA);
	}

	void CPU::CheckForInterrupt()
	{
		if (ProcessorStatus & (uint8)PFlags::INTERRUPT_DISABLED)
		{
			//return;
		}

		uint8 ppuCtrl = ReadMemory8(PPU_CTRL_ADR);

		// if bit 7 set of ppu ctrl skip nmi. 
		// if (ppuCtrl & (uint8)PPUCtrl::VerticalBlanking)
		if (m_nmi && (ppuCtrl & (uint8)PPUCtrl::VerticalBlanking))
		{
			PushStack16(PC);
			PushStack8(ProcessorStatus);
			PC = ReadMemory16(0xFFFA);
			m_nmi = false;
		}
	}

	void CPU::DebugOutput()
	{
	}

	void CPU::Update()
	{
		// Check for non-maskable interrupt
		CheckForInterrupt();
		uint8 opCode = ReadMemory8(PC);
		m_instructions[opCode]->Execute(opCode);
	}

	void CPU::UpdateInput()
	{
		SDL_Event _event;
		SDL_Scancode key;
		const Uint8* keyState = SDL_GetKeyboardState(NULL);
		while (SDL_PollEvent(&_event))
		{
			if (keyState[SDL_SCANCODE_ESCAPE] || _event.type == SDL_QUIT) {
			}

			switch (_event.type) {
			case SDL_KEYDOWN:
				key = _event.key.keysym.scancode;

				if (key == SDL_SCANCODE_X)
				{
					m_controller1Input |= (uint8)Controller::A;
				}
				if (key == SDL_SCANCODE_C)
				{
					m_controller1Input |= (uint8)Controller::B;
				}
				if (key == SDL_SCANCODE_LEFT)
				{
					m_controller1Input |= (uint8)Controller::LEFT;
				}
				if (key == SDL_SCANCODE_RIGHT)
				{
					m_controller1Input |= (uint8)Controller::RIGHT;
				}
				if (key == SDL_SCANCODE_UP)
				{
					m_controller1Input |= (uint8)Controller::UP;
				}
				if (key == SDL_SCANCODE_DOWN)
				{
					m_controller1Input |= (uint8)Controller::DOWN;
				}
				if (key == SDL_SCANCODE_RETURN)
				{
					m_controller1Input |= (uint8)Controller::START;
				}
				if (key == SDL_SCANCODE_0)
				{
					m_controller1Input |= (uint8)Controller::SELECT;
				}
				break;
			case SDL_KEYUP:
				key = _event.key.keysym.scancode;

				if (key == SDL_SCANCODE_X)
				{
					m_controller1Input &= ~(uint8)Controller::A;
				}
				if (key == SDL_SCANCODE_C)
				{
					m_controller1Input &= ~(uint8)Controller::B;
				}
				if (key == SDL_SCANCODE_LEFT)
				{
					m_controller1Input &= ~(uint8)Controller::LEFT;
				}
				if (key == SDL_SCANCODE_RIGHT)
				{
					m_controller1Input &= ~(uint8)Controller::RIGHT;
				}
				if (key == SDL_SCANCODE_UP)
				{
					m_controller1Input &= ~(uint8)Controller::UP;
				}
				if (key == SDL_SCANCODE_DOWN)
				{
					m_controller1Input &= ~(uint8)Controller::DOWN;
				}
				if (key == SDL_SCANCODE_RETURN)
				{
					m_controller1Input &= ~(uint8)Controller::START;
				}
				if (key == SDL_SCANCODE_0)
				{
					m_controller1Input &= ~(uint8)Controller::SELECT;
				}
				break;
			}
		}

#ifdef PSVITA
		SceCtrlData ctrl;
		sceCtrlPeekBufferPositive(0, &ctrl, 1);

		SetControllerInput((uint8)Controller::START, ctrl.buttons & SCE_CTRL_START);
		SetControllerInput((uint8)Controller::SELECT, ctrl.buttons & SCE_CTRL_SELECT);
		SetControllerInput((uint8)Controller::A, ctrl.buttons & SCE_CTRL_CROSS);
		SetControllerInput((uint8)Controller::B, ctrl.buttons & SCE_CTRL_CIRCLE);
		SetControllerInput((uint8)Controller::UP, ctrl.buttons & SCE_CTRL_UP);
		SetControllerInput((uint8)Controller::DOWN, ctrl.buttons & SCE_CTRL_DOWN);
		SetControllerInput((uint8)Controller::LEFT, ctrl.buttons & SCE_CTRL_LEFT);
		SetControllerInput((uint8)Controller::RIGHT, ctrl.buttons & SCE_CTRL_RIGHT);
#endif
	}

	void CPU::SetControllerInput(uint8 input, bool down)
	{
		if (down)
		{
			m_controller1Input |= input;
		}
		else
		{
			m_controller1Input &= ~input;
		}
	}

	void CPU::LoadCartridge(Cartridge* cartridge)
	{
		// Does not support dynamic bank loading at present, games with more than 2 banks will not work
		m_loadedCartridge = cartridge;

		// Default - Load the first rom bank into $8000 and last rom bank into $C000
		const std::vector<uint8>& bank0 = cartridge->GetPRGRomBank(0);
		const std::vector<uint8>& bank1 = cartridge->GetPRGRomBank(cartridge->GetNumPRGRomBanks() - 1);

		// Copy banks to PRGROM
		for (int i = 0; i < bank0.size(); ++i)
		{
			RAM[PRGROM_LOWER + i] = bank0[i];
			RAM[PRGROM_UPPER + i] = bank1[i];
		}

		// load pattern tables into PPU
		if (cartridge->GetNumVRamBanks() > 0)
		{
			const std::vector<uint8> vrambank0 = cartridge->GetCHRBank(0);

			for (int i = 0; i < vrambank0.size(); ++i)
			{
				m_ppu->WriteMemory8(i, vrambank0[i]);
			}
		}

		if (cartridge->GetNumVRamBanks() > 1)
		{
			const std::vector<uint8> vrambank1 = cartridge->GetCHRBank(1);

			for (int i = 0; i < vrambank1.size(); ++i)
			{
				m_ppu->WriteMemory8(0x1FFF + i, vrambank1[i]);
			}
		}

		// Set the program counter to the reset vector 
		PC = ReadMemory16(0xFFFC);
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

	uint8 buffered = 0;

	bool scrollmodefirstwrite = true;

	/** CPU MEMORY READ & WRITE **/
	void CPU::WriteMemory8(uint16 Addr, uint8 data)
	{
		if (m_startup)
		{
			if (m_cycleCounter < 29658)
			{
				if (Addr == PPU_DATA_ADR || Addr == PPU_MASK_ADR || Addr == PPU_COARSE_SCROLL_ADR || Addr == PPU_SCROLL_ADR)
				{
					return;
				}
			}
		}

		if (Addr >= 0x4000 && Addr <= 0x4010) {
			//printf("wow\n");
		}


		if (Addr == CONTROLLER1_ADR)
		{
			if (data == 0x1)
			{
				m_controllerLatched = true;
			}
		}

		if (Addr == 0x2002 || Addr == 0x2003)
		{
			RAM[PPU_STATUS_ADR] = (RAM[PPU_STATUS_ADR] & 0x1F) | (data & 0xE0);
		}
		// Write lsb of data previous written into PPU registers into ppu status $2002 register
		else if ((Addr >= PPU_CTRL_ADR && Addr < 0x2005) || Addr == OAM_DMA_ADR)
		{
			// Write lsb (5 bits) into PPUSTATUS register $2002
			//RAM[PPU_STATUS_ADR] = (RAM[PPU_STATUS_ADR] & 0xE0) | (Data & 0x1F);
			RAM[PPU_STATUS_ADR] |= data;

			if (Addr == 0x2000)
			{
				RAM[Addr] = data;
			}

			//if (Addr == 0x2007 || Addr == 0x2006 || Addr == 0x2005)
			//{
			//	RAM[Addr] = Data;
			//}
		}
		else
		{
			RAM[Addr] = data;
		}

		// Write to PPUDATA $2700 - VRAM Address incremented by bit 2 of $2000 (cpu ctrl address) after read/ write
		if (Addr == PPU_DATA_ADR)// || Addr == PPU_SCROLL_ADR) // maybe not
		{
			m_ppu->WriteMemory8(m_vramAddress, data);

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

		if (Addr == PPU_SCROLL_ADR)
		{
			if (scrollmodefirstwrite)
			{
				m_ppu->m_scrollX = 0xE0 & data;
			}
			else
			{
				m_ppu->m_scrollY = 0xE0 & data;
			}

			printf("%i\n", m_ppu->m_scrollX);

			scrollmodefirstwrite = !scrollmodefirstwrite;
		}

		// Write OAM Address 
		if (Addr == OAM_ADR)
		{
			m_oamAddress = data;
		}

		// Writes to OAM data $2004, increment OAMADDR after the write, reads during vertial or forced blanking return the value but don't increment
		if (Addr == OAM_DATA_ADR)
		{
			m_ppu->WriteOAMByte(m_oamAddress, data);
			m_oamAddress++;
		}

		// OAM DMA $4014 - Initialise DMA
		if (Addr == OAM_DMA_ADR)
		{
			uint16 start = 0x100 * data;
			for (uint offset = 0; offset <= 0xFF; ++offset)
			{
				m_ppu->WriteOAMByte(offset, ReadMemory8(start + offset));
			}

			m_cycleCounter += 513;
			m_cycleCounter += m_cycleCounter % 2;
			m_oamAddress = 0xFF;
		}

		// $2006/ $2005? maybe not
		if (Addr == PPU_COARSE_SCROLL_ADR)// || Addr == PPU_SCROLL_ADR)
		{
			if (!m_vramToggle)
			{
				m_vramAddress = (uint16)data << 8;
			
				//Lower 5 bits.
				m_ppu->m_scrollX = 0x1F & data;
				
				// upper 3 bits.
				m_ppu->m_scrollY = 0xE0 & data;
				//
				//printf("%i\n", m_ppu->m_coarseX);
			}
			else
			{
				m_vramAddress |= data;
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
				this->RAM[Addr + 0x0800] = data;
				this->RAM[Addr + 0x1000] = data;
				this->RAM[Addr + 0x1800] = data;
				return;
			}
			else if (Addr < 0x1000)
			{
				this->RAM[Addr - 0x0800] = data;
				this->RAM[Addr + 0x0800] = data;
				this->RAM[Addr + 0x1000] = data;
				return;
			}
			else if (Addr < 0x1800)
			{
				this->RAM[Addr - 0x1000] = data;
				this->RAM[Addr - 0x0800] = data;
				this->RAM[Addr + 0x0800] = data;
				return;
			}
		}

		//!< Writes to I/O register range $2000-$2007 are mirrored 
		//!< every 8 bytes in the range $2008-$3FFF

		//!< Initially check if within range or I/O regs and mirrors 
		if ((Addr < 0x4000) && (Addr >= 0x2000))
		{
			//!< Minus the starting position and get the remainder of the addr / 8 in order to establish 
			//!< the bytes that should be mirrored (mByte -> byte to be mirrored)
			uint16 mByte = (Addr - 0x2000) % 8;

			if (mByte > 7)
			{
				RAM[0x2000 + mByte] = data;
			}

			////!< For every byte in range $2000 - $3FFF (kinda) 
			//for (uint16 _m = mByte; _m < 0x4000; _m += 8) {
			//	this->RAM[0x2000 + _m] = Data;
			//}
		}
	}

	uint8 CPU::ReadMemory8(uint16 Addr)
	{
		if (Addr == CONTROLLER1_ADR || Addr == CONTROLLER2_ADR)
		{
			if (m_controllerLatched)
			{
				uint8 input = (Addr == CONTROLLER1_ADR) ? m_controller1Input : m_controller2Input;

				// read input 1 bit at time and increment 
				input = (input >> m_controllerReadBit) & 0x1;
				m_controllerReadBit++;

				if (m_controllerReadBit >= 8)
				{
					m_controllerReadBit = 0;

					if (Addr == CONTROLLER2_ADR)
					{
						m_controllerLatched = false;
					}
				}
				//WriteMemory8(Addr, input);
				return input;
			}
		}

		if (Addr == PPU_DATA_ADR || Addr == PPU_SCROLL_ADR)
		{
			uint8 data = m_ppu->ReadMemory8(m_vramAddress, true);

			uint8 incrememnt = (RAM[PPU_CTRL_ADR] & (uint8)PPUCtrl::VRamAddressIncrement);
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

			return data;
		}

		// When a read from $2002 occurs, bit 7 is reset to 0 as are $2005 and $2006.
		if (Addr == PPU_STATUS_ADR)
		{
			//RAM[PPU_STATUS_ADR] &= ~(uint8)PPUStatus::VerticalBlank;
			////RAM[PPU_STATUS_ADR] &= ~(uint8)PPUStatus::Sprite0Hit;
			//RAM[0x2005] = 0;
			//RAM[0x2006] = 0;
			m_vramToggle = false;
			scrollmodefirstwrite = false;
		}

		// Handle memory mirrored between $2000-$3FFF
		if ((Addr < 0x4000) && (Addr >= 0x2000))
		{
			uint16 mByte = (Addr - 0x2000) % 8;
			return RAM[0x2000 + mByte];
		}

		return RAM[Addr];
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
	void CPU::BRK_$00(AdrMode Mode)
	{
		PushStack16(PC);
		PushStack8(ProcessorStatus);
		SetProcessorFlag(PFlags::BRK_CMD, true);
		PC = ReadMemory16(0xFFFE);
	}
	void CPU::CLC_$18(AdrMode Mode)
	{
		SetProcessorFlag(PFlags::CARRY, false);
		PC++;
	}
	void CPU::CLD_$D8(AdrMode Mode)
	{
		SetProcessorFlag(PFlags::DECIMAL_MODE, false);
		PC++;
	}
	void CPU::CLI_$58(AdrMode Mode)
	{
		SetProcessorFlag(PFlags::INTERRUPT_DISABLED, false);
		PC++;
	}
	void CPU::CLV_$B8(AdrMode Mode)
	{
		SetProcessorFlag(PFlags::OVER_FLOW, false);
		PC++;
	}
	void CPU::DEX_$CA(AdrMode Mode)
	{
		XReg--;
		SetProcessorFlag(PFlags::ZERO, XReg == 0);
		SetProcessorFlag(PFlags::NEGATIVE, XReg & PFlags::NEGATIVE);
		PC++;
	}
	void CPU::DEY_$88(AdrMode Mode)
	{
		YReg--;
		SetProcessorFlag(PFlags::ZERO, YReg == 0);
		SetProcessorFlag(PFlags::NEGATIVE, YReg & PFlags::NEGATIVE);
		PC++;
	}
	void CPU::INX_$E8(AdrMode Mode)
	{
		XReg++;
		SetProcessorFlag(PFlags::ZERO, XReg == 0);
		SetProcessorFlag(PFlags::NEGATIVE, XReg & PFlags::NEGATIVE);
		PC++;
	}
	void CPU::INY_$C8(AdrMode Mode)
	{
		YReg++;
		SetProcessorFlag(PFlags::ZERO, YReg == 0);
		SetProcessorFlag(PFlags::NEGATIVE, YReg & PFlags::NEGATIVE);
		PC++;
	}
	void CPU::NOP_$EA(AdrMode Mode)
	{
		//!< Does nothing 
		PC++;
	}
	void CPU::PHA_$48(AdrMode Mode)
	{
		PushStack8(Accumulator);
		PC++;
	}
	void CPU::PHP_$08(AdrMode Mode)
	{
		PushStack8(ProcessorStatus);
		PC++;
	}
	void CPU::PLA_$68(AdrMode Mode)
	{
		Accumulator = PopStack8();
		SetProcessorFlag(PFlags::ZERO, Accumulator == 0);
		SetProcessorFlag(PFlags::NEGATIVE, Accumulator & PFlags::NEGATIVE);
		PC++;
	}
	void CPU::PLP_$28(AdrMode Mode)
	{
		ProcessorStatus = PopStack8();
		PC++;
	}
	void CPU::RTI_$40(AdrMode Mode)
	{
		ProcessorStatus = PopStack8();
		PC = PopStack16();
	}
	void CPU::JSR_$20(AdrMode Mode)
	{
		PC++;
		uint16 jmpAdr = ReadMemory16(PC);
		PushStack16(PC + 1);
		PC = jmpAdr;
	}
	void CPU::RTS_$60(AdrMode Mode)
	{
		this->PC = PopStack16() + 1;
	}
	void CPU::SEC_$38(AdrMode Mode)
	{
		this->ProcessorStatus |= PFlags::CARRY;
		PC++;
	}
	void CPU::SED_$f8(AdrMode Mode) {
		this->ProcessorStatus |= PFlags::DECIMAL_MODE;
		PC++;
	}
	void CPU::SEI_$78(AdrMode Mode) {
		this->ProcessorStatus |= PFlags::INTERRUPT_DISABLED;
		PC++;
	}
	void CPU::TAX_$AA(AdrMode Mode)
	{
		XReg = Accumulator;
		SetProcessorFlag(PFlags::ZERO, XReg == 0);
		SetProcessorFlag(PFlags::NEGATIVE, XReg & PFlags::NEGATIVE);
		PC++;
	}
	void CPU::TAY_$A8(AdrMode Mode) {
		YReg = Accumulator;
		SetProcessorFlag(PFlags::ZERO, YReg == 0);
		SetProcessorFlag(PFlags::NEGATIVE, YReg & PFlags::NEGATIVE);
		PC++;
	}
	void CPU::TSX_$BA(AdrMode Mode) {
		this->XReg = this->SP;
		SetProcessorFlag(PFlags::ZERO, XReg == 0);
		SetProcessorFlag(PFlags::NEGATIVE, XReg & PFlags::NEGATIVE);
		PC++;
	}
	void CPU::TXA_$8A(AdrMode Mode) {
		this->Accumulator = this->XReg;
		SetProcessorFlag(PFlags::ZERO, Accumulator == 0);
		SetProcessorFlag(PFlags::NEGATIVE, Accumulator & PFlags::NEGATIVE);
		PC++;
	}
	void CPU::TXS_$9A(AdrMode Mode)
	{
		this->SP = this->XReg;
		PC++;
	}
	void CPU::TYA_$98(AdrMode Mode) {
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
			uint16 msb = ReadMemory8(PC + 1);

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
		PC += 2;
		uint8 M = ReadMemory8(PC - 1);

		if (M & 0x80)
		{
			M &= ~0x80;
			return PC - (128 - M);
		}
		else
		{
			return PC + M;
		}
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
