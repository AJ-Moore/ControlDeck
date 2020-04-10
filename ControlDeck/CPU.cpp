//!< Author Allan Moore 20/ 03/ 2015 

#include "CPU.h"

namespace ControlDeck {

	CPU::CPU()
	{
		//!< The 6502 uses a 16 bit address bus $0 - $FFFF ($FFFF+1 possible addresses) 
		this->RAM = new UByte[0xFFFF]; //!< err $FFFF invalid, wrap to 0?

		//!< Initialise RAM to NAUGHT
		for (uint16 i = 0; i < 0xFFFF; i++)
		{
			this->RAM[i] = 0;
		}

		//!< NOTE TO SELF : Some initialisations may want to be moved over or mirrored/ duplicated 
		//!<				in 'reset' in order to ensure consistency between rom switching and
		//!<				'hardware' resets

		//!< Set stack pointer to FF --> notably this could be any concievable value between 0 and FF 
		//!< because it wraps so I couldn't see any concievable difference why the start position would 
		//!< matter?
		this->SP = 0xFF;

		//!< Set program counter initially to Upper PRGROM-Bank 
		this->PC = this->PRGROM_UPPER;
	}

	CPU::~CPU() {

	}

	void CPU::Initialise() {

	}

	void CPU::Update() {

		//?? Some Unknown Code Regarding CPU cycles should be thought out and put here 

		//Switch (ProgramCounter) -> Call instructions -> probably faster than using function pointers for each operation code
		while (true)
		{//!< While something or other execute instructions
			uint8 opCode = this->ReadMemory8(this->PC);

			//!< Switch instructions
			switch (opCode) {
			default:
				//!< Whats the default case?
				break;
			}

			//Increment the program counter 
			this->PC++;
		}
	}

	void CPU::SetProcessorFlag(PFlags Flag, bool bEnabled)
	{
		if (bEnabled)
		{
			ProcessorStatus |= Flag;
		}
		else
		{
			ProcessorStatus & ~(Flag);
		}
	}

	/** CPU MEMORY READ & WRITE **/
	void CPU::WriteMemory8(uint16 Addr, uint8 Data) {

		//!< Not sure what to do here yet -> just so I don't forget
		if (Addr == 0xFFFF) {
			//throw eMAOOR;
		}

		//!< * Memory at $000-$07FF mirrored at $0800, $1000, $1800
		//!< * Assuming any data written to range $0800-$2000 should
		//!< * Mirroring will occur appriopiately 
		if (Addr < 0x2000) {
			if (Addr < 0x0800) {
				//!< Mirroring at $0800, $1000 and $1800 
				this->RAM[Addr] = Data;
				this->RAM[Addr + 0x0800] = Data;
				this->RAM[Addr + 0x1000] = Data;
				this->RAM[Addr + 0x1800] = Data;
				return;
			}
			else if (Addr < 0x1000) {
				this->RAM[Addr - 0x0800] = Data;
				this->RAM[Addr] = Data;
				this->RAM[Addr + 0x0800] = Data;
				this->RAM[Addr + 0x1000] = Data;
				return;
			}
			else if (Addr < 0x1800) {
				this->RAM[Addr - 0x1000] = Data;
				this->RAM[Addr - 0x0800] = Data;
				this->RAM[Addr] = Data;
				this->RAM[Addr + 0x0800] = Data;
				return;
			}
		}

		//!< Writes to I/O register range $2000-$2007 are mirrored 
		//!< every 8 bytes in the range $2008-$3FFF

		//!< Initially check if within range or I/O regs and mirrors 
		if (Addr < 0x4000) {
			//!< Minus the starting position and get the remainder of the addr / 8 in order to establish 
			//!< the bytes that should be mirrored (mByte -> byte to be mirrored)
			uint8 _mByte = (Addr - 0x2000) % 8;

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
		mem &= (ReadMemory8(Addr + 1) << 8);
		return mem;
	}

	/** ======================= **/

#pragma region _6502_Instructions 

#pragma region Implied Addressing Mode Instructions
/***Implied Addressing Mode Instructions (25)***/
	void CPU::BRK_$00() {
		this->PC++;
		WriteMemory8(this->STACK + this->SP, PC >> 8);
		this->DecrementSP();
		WriteMemory8(this->STACK + this->SP, PC);
		this->DecrementSP();

	}
	void CPU::CLC_$18() {
		this->ProcessorStatus &= ~(PFlags::CARRY);
	}
	void CPU::CLD_$D8() {
		this->ProcessorStatus &= ~(PFlags::DECIMAL_MODE);
	}
	void CPU::CLI_$58() {
		this->ProcessorStatus &= ~(PFlags::INTERRUPT_DISABLED);
	}
	void CPU::CLV_$B8() {
		this->ProcessorStatus &= ~(PFlags::OVER_FLOW);
	}
	void CPU::DEX_$CA() {
		this->XReg--;

		// If result of decriment is zero set zero flag 
		(this->XReg == 0) ? this->PC |= PFlags::ZERO : this->PC &= ~PFlags::ZERO;

		// If bit 7 is set, set negative flag 
		(this->XReg & (1 << 7)) ? this->PC |= PFlags::NEGATIVE : this->PC &= ~PFlags::NEGATIVE;
	}
	void CPU::DEY_$88() {
		this->YReg--;

		// If result of decriment is zero set zero flag 
		(this->YReg == 0) ? this->PC |= PFlags::ZERO : this->PC &= ~PFlags::ZERO;

		// If bit 7 is set, set negative flag 
		(this->YReg & (1 << 7)) ? this->PC |= PFlags::NEGATIVE : this->PC &= ~PFlags::NEGATIVE;
	}
	void CPU::INX_$E8() {
		this->XReg++;

		// If result of decriment is zero set zero flag 
		(this->XReg == 0) ? this->PC |= PFlags::ZERO : this->PC &= ~PFlags::ZERO;

		// If bit 7 is set, set negative flag 
		(this->XReg & (1 << 7)) ? this->PC |= PFlags::NEGATIVE : this->PC &= ~PFlags::NEGATIVE;
	}
	void CPU::INY_$C8() {
		this->YReg++;

		// If result of decriment is zero set zero flag 
		(this->YReg == 0) ? this->PC |= PFlags::ZERO : this->PC &= ~PFlags::ZERO;

		// If bit 7 is set, set negative flag 
		(this->YReg & (1 << 7)) ? this->PC |= PFlags::NEGATIVE : this->PC &= ~PFlags::NEGATIVE;
	}
	void CPU::NOP_$EA() {
		//!< Does nothing 
	}
	void CPU::PHA_$48() {
		WriteMemory8(this->STACK + this->SP, this->Accumulator);
		this->DecrementSP();
	}
	void CPU::PHP_$08() {
		WriteMemory8(this->STACK + this->SP, this->ProcessorStatus);
		this->DecrementSP();
	}
	void CPU::PLA_$68() {
		this->IncrementSP();
		this->Accumulator = ReadMemory8(this->STACK + this->SP);

		(this->Accumulator == 0) ? this->ProcessorStatus |= PFlags::ZERO : this->ProcessorStatus &= ~PFlags::ZERO;
		(this->Accumulator & (1 << 7)) ? this->ProcessorStatus |= PFlags::NEGATIVE : this->ProcessorStatus &= ~PFlags::NEGATIVE;
	}
	void CPU::PLP_$28() {
		this->IncrementSP();
		this->ProcessorStatus = ReadMemory8(this->STACK + this->SP);
	}
	void CPU::RTI_$40() {
		this->IncrementSP();
		this->ProcessorStatus = ReadMemory8(this->STACK + this->SP);

		//!< Get/ restore? the instructions address from memory ??
		//!< Needs Checking
		this->IncrementSP();
		uint16 _l = ReadMemory8(this->STACK + this->SP);
		this->IncrementSP();
		uint16 _h = ReadMemory8(this->STACK + this->SP) << 8;

		this->PC = _h | _l;

	}
	void CPU::RTS_$60() {
		this->IncrementSP();
		uint16 _l = ReadMemory8(this->STACK + this->SP);
		this->IncrementSP();
		uint16 _h = ReadMemory8(this->STACK + this->SP) << 8;
		this->PC = _h | _l;
	}
	void CPU::SEC_$38() {
		this->ProcessorStatus |= PFlags::CARRY;
	}
	void CPU::SED_$f8() {
		this->ProcessorStatus |= PFlags::DECIMAL_MODE;
	}
	void CPU::SEI_$78() {
		this->ProcessorStatus |= PFlags::INTERRUPT_DISABLED;
	}
	void CPU::TAX_$AA() {
		this->XReg = this->Accumulator;
		(this->XReg == 0) ? this->ProcessorStatus |= PFlags::ZERO : this->ProcessorStatus &= ~PFlags::ZERO;
		(this->XReg & (1 << 7)) ? this->ProcessorStatus |= PFlags::NEGATIVE : this->ProcessorStatus &= ~PFlags::NEGATIVE;

	}
	void CPU::TAY_$A8() {
		this->YReg = this->Accumulator;
		(this->YReg == 0) ? this->ProcessorStatus |= PFlags::ZERO : this->ProcessorStatus &= ~PFlags::ZERO;
		(this->YReg & (1 << 7)) ? this->ProcessorStatus |= PFlags::NEGATIVE : this->ProcessorStatus &= ~PFlags::NEGATIVE;
	}
	void CPU::TSX_$BA() {
		this->XReg = this->SP;
		(this->XReg == 0) ? this->ProcessorStatus |= PFlags::ZERO : this->ProcessorStatus &= ~PFlags::ZERO;
		(this->XReg & (1 << 7)) ? this->ProcessorStatus |= PFlags::NEGATIVE : this->ProcessorStatus &= ~PFlags::NEGATIVE;
	}
	void CPU::TXA_$8A() {
		this->Accumulator = this->XReg;
		(this->Accumulator == 0) ? this->ProcessorStatus |= PFlags::ZERO : this->ProcessorStatus &= ~PFlags::ZERO;
		(this->Accumulator & (1 << 7)) ? this->ProcessorStatus |= PFlags::NEGATIVE : this->ProcessorStatus &= ~PFlags::NEGATIVE;
	}
	void CPU::TXS_$9A() {
		this->SP = this->XReg;
	}
	void CPU::TYA_$98() {
		this->Accumulator = this->YReg;
		(this->Accumulator == 0) ? this->ProcessorStatus |= PFlags::ZERO : this->ProcessorStatus &= ~PFlags::ZERO;
		(this->Accumulator & (1 << 7)) ? this->ProcessorStatus |= PFlags::NEGATIVE : this->ProcessorStatus &= ~PFlags::NEGATIVE;
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
	void CPU::ADC_$69()//!< Immediate 
	{
		ADC(AdrMode::IMMEDIATE);
	}
	void CPU::ADC_$65()//!<Zero page 
	{
		ADC(AdrMode::ZERO_PAGE);
	}
	void CPU::ADC_$75()//!< Indexed Zero Page, X	
	{
		ADC(AdrMode::ZERO_PAGEX);
	}
	void CPU::ADC_$6D()//!< Absolute (16 bit adr)
	{
		ADC(AdrMode::ABSOLUTE);
	}
	void CPU::ADC_$7D()//!< Indexed Absolute X (16 bit adr)
	{
		ADC(AdrMode::ABSOLUTEX);
	}
	void CPU::ADC_$79() //!< Indexed Absolute Y (16 bit adr)
	{
		ADC(AdrMode::ABSOLUTEY);
	}
	void CPU::ADC_$61() //!< Indexed Indirect X 
	{
		ADC(AdrMode::INDEXED_INDIRECT);
	}
	void CPU::ADC_$71() //!< Indirect Indixed Y
	{
		ADC(AdrMode::INDIRECT_INDEXED);
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
	void CPU::AND_$29()
	{
		AND(AdrMode::IMMEDIATE);
	}
	void CPU::AND_$25()
	{
		AND(AdrMode::ZERO_PAGE);
	}
	void CPU::AND_$35()
	{
		AND(AdrMode::ZERO_PAGEX);
	}
	void CPU::AND_$2D()
	{
		AND(AdrMode::ABSOLUTE);
	}
	void CPU::AND_$3D()
	{
		AND(AdrMode::ABSOLUTEX);
	}
	void CPU::AND_$39()
	{
		AND(AdrMode::ABSOLUTEY);
	}
	void CPU::AND_$21()
	{
		AND(AdrMode::INDEXED_INDIRECT);
	}
	void CPU::AND_$31()
	{
		AND(AdrMode::INDIRECT_INDEXED);
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

	void CPU::BIT(AdrMode Mode)
	{

	}

	void CPU::CMP(AdrMode Mode)
	{

	}

	void CPU::CPX(AdrMode Mode)
	{

	}

	void CPU::CPY(AdrMode Mode)
	{

	}

	void CPU::DEC(AdrMode Mode)
	{

	}

	void CPU::EOR(AdrMode Mode)
	{

	}

	void CPU::INC(AdrMode Mode)
	{

	}

	void CPU::JMP(AdrMode Mode)
	{

	}

	void CPU::ORA(AdrMode Mode)
	{

	}

	void CPU::ROL(AdrMode Mode)
	{

	}

	void CPU::ROR(AdrMode Mode)
	{

	}

	void CPU::SBC(AdrMode Mode)
	{

	}

	void CPU::STA(AdrMode Mode)
	{

	}

	void CPU::JSR_$20()
	{

	}

	void CPU::LDA()
	{

	}

	void CPU::LDX()
	{

	}

	void CPU::LDY()
	{

	}

	void CPU::LSR()
	{

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
		uint16 Adr = ReadMemory16(ReadMemory16(PC));
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
		PC++;
		uint16 M = (uint16)ReadMemory8(PC) + PC;
		PC++;
		return M;
	}

	uint16 CPU::GetMemIndexedIndirect()
	{
		PC++;
		uint16 Adr = ReadMemory16(ReadMemory16(PC) + XReg);
		PC += 2;
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