/*
	Author: Allan Moore
*/

/*
	Credit to the talented guys and years of research/ documentation
	available at http://nesdev.com/
*/

/*********************************************************************
	* Class: CPU
	* --------------------------------------------------------------
	* Simulates the operation of the 2A03/ 2A07 8 bit microprocessor
	* developed by Ricoh for use in the nintendo entertainment
	* system(Famicom) developed by Nintendo, between 1982 and 1994.
	* --------------------------------------------------------------
	*
	* CPU Specification/ Info
	* --------------------------------------------------------------
	* From the 6502 processor family
	*
	* Unlike the 6502 features a audio processing unit
	*		pAPU - pseudo Audio Processing Unit
	*
	* Lacks support for BCD found on the 6502.
	*
	* Uses a 16 bit address bus, valid addresses between ranges:
	* $0000 - $FFFF
	*
	* 64kb of available memory
**********************************************************************/

/* CPU Memory Map

$10000
	PRG-ROM
		* Allocated for cartridge PRG-ROM from $8000
		* Games with only a single 16kb bank will load into both 8000 & C000 effectively mirrored
		* Games with 2 16 kb bank will load one into $8000 and one into $C000
	- PRG-ROM Upper Bank $C000 - $10000
	- PRG-ROM Lower Bank $8000 - $C000
$8000
	SRAM

$6000
	Expansion ROM

$4020
	Memory Mapped I/O Registers (Same address bus to address memory and I/O) Not sure of benefits of Memory Mapped I/O over Port Mapped I/O)
	 - I/O Registers $4000 - $4020
	 - Mirrors $2008-$3FFF
		* Mirrors $2000 - $2007 for every 8 bytes
	 - I/O $2000-$2007

$2000
	RAM
	 - Mirrors $000-$07FF
		* Memory at $000-$07FF mirrored at $0800, $1000, $1800
		* i.e. RAM, Stack, Zero Page below mirrored here
	 - RAM $0200 - $07FF
	 - Stack $0100 - $01FF
	 - Zero Page $0000 - $00FF
		* Zero page addresses/ for faster read/write
		* Special space designated at Zero address space
		* Naturally 256 bytes 0 - FF
$0000

*/

#ifndef CPU_6502_H 
#define CPU_6502_H

#include "AddressingMode.h"
#include "Common.h"
#include "ProcessorStatusFlags.h"

namespace ControlDeck
{
	class CPU
	{
	public:
		CPU();
		~CPU();

		void Initialise();

		//!< Fetch next instruction and execute 
		void Update();

		void SetProcessorFlag(PFlags Flag, bool bEnabled);
		uint8 ReadMemory8(uint16 Addr);
		uint16 ReadMemory16(uint16 Addr);

		//!< Used for writing U8 to particular memory address 
		//!< We use a writeMemory function in order to ensure that data is
		//!< mirrored correctly... basically 
		void WriteMemory8(uint16 Addr, uint8 Data);
		//!< Haven't decide on whether necessary to have an additional function 
		//!< for writing memory in case where operand makes use of 16 bits of 
		//!< data... considering most data is actually handled in 8 bits.. meh
		//!< I'll decide as and when necessary!
		//void writeMemory16(U16 Addr, U16 Data);

/** Useful Constants - STACK(0x0100), PRGROM_UPPER(0xC000),PRGROM_LOWER(0x8000) **/
		//!< The location from which the stack begins
		const uint16 STACK = 0x0100;

		//!< Location of PRG ROM Upper Bank 
		const uint16 PRGROM_UPPER = 0xC000;

		//!< Location of PRG ROM Lower Bank
		const uint16 PRGROM_LOWER = 0x8000;

	private:

		//!< Insert about 151 instructions here D:
#pragma region _6502_Instructions 

#pragma region _Implied_Addressing_Mode_Instructions (25 instructions)
/***Implied Addressing Mode Instructions***/
	//!< Software Interrupt 
		void BRK_$00();

		//!< Clears Carry Flag C
		void CLC_$18();

		//!< Clear Decimal Flag D :: Note Decimal Mode not supported on 2A03/07 CPU
		void CLD_$D8();

		//!< Clear Intterupt Disable Flag I
		void CLI_$58();

		//!< Clear Overflow Flag V
		void CLV_$B8();

		//!< Decrement X register by one 
		void DEX_$CA();

		//!< Decrement Y register by one 
		void DEY_$88();

		//!< Increment X register by one
		void INX_$E8();

		//!< Increment Y register by one 
		void INY_$C8();

		//!< Does Nothing :D
		void NOP_$EA();

		//!< Push (A) Accumulator onto stack
		void PHA_$48();

		//!< Push Processor Status onto stack 
		void PHP_$08();

		//!< Pull from stack to (A) Accumulator
		void PLA_$68();

		//!< Pull from stack to (P) Processor Status
		void PLP_$28();

		//!< Return from interrupt
		void RTI_$40();

		//!< Return from Subroutine 
		void RTS_$60();

		//!< Set Carry Flag C
		void SEC_$38();

		//!< Set Decimal Flag D
		void SED_$f8();

		//!< Set Intterupt Disabled Flag
		void SEI_$78();

		//!< Transfer Accumulator to X register 
		void TAX_$AA();

		//!< Transfer Accumulator to Y register
		void TAY_$A8();

		//!< Transfer Stack Pointer to X register
		void TSX_$BA();

		//!< Transfer X register to Accumulator 
		void TXA_$8A();

		//!< Transfer X register to Stack Pointer 
		void TXS_$9A();

		//!< Tranfer Y register to Accumulator 
		void TYA_$98();
#pragma endregion 

		/*Add memory to Accumulator with carry*/
		void ADC(AdrMode Mode);
		void ADC_$69();	//!< Immediate 
		void ADC_$65(); //!< Zero Page
		void ADC_$75(); //!< Indexed Zero Page
		void ADC_$6D();	//!< Absolute (16 bit adr)
		void ADC_$7D(); //!< Indexed Absolute X (16 bit adr)
		void ADC_$79(); //!< Indexed Absolute Y (16 bit adr)
		void ADC_$61(); //!< Indexed Indirect X 
		void ADC_$71(); //!< Indirect Indixed Y

	/*AND - Bitwsie AND */
		void AND(AdrMode Mode);
		void AND_$29(); //!< Immediate
		void AND_$25(); //!< Zero Page 
		void AND_$35(); //!< Zero Page, X
		void AND_$2D(); //!< Absolute 
		void AND_$3D(); //!< Absolute, X 
		void AND_$39(); //!< Absolute, Y 
		void AND_$21(); //!< Indexed Indirect X
		void AND_$31(); //!< Indirect Indexed Y

	/*Shift Left*/
		void ASL(AdrMode Mode);
		void ASL_$0A(); //!< Accumulator
		void ASL_$06(); //!< Zero Page 
		void ASL_$16(); //!< Zero Page, X
		void ASL_$0E(); //!< Absolute 
		void ASL_$1E(); //!< Absolute, X

	/*BIT - Test b*/
		void BIT(AdrMode Mode);
		void BIT_$24(); //!< Zero Page 
		void BIT_$2C(); //!< Absolute

	/*CMP = Compare Acumulator with Memory*/
		void CMP(AdrMode Mode);
		void CMP_$C9(); //!< Immediate
		void CMP_$C5(); //!< Zero Page
		void CMP_$D5(); //!< Zero Page, X
		void CMP_$CD(); //!< Absolute 
		void CMP_$DD(); //!< Absolute, X 
		void CMP_$D9(); //!< Absolute, Y 
		void CMP_$C1(); //Indexed Indirect X 
		void CMP_$D1(); //Indirect Indexed Y

	/*CPX - Comapre X with Memory*/
		void CPX(AdrMode Mode);
		void CPX_$E0(); //!< Immediate
		void CPX_$E4(); //!< Zero Page 
		void CPX_$EC(); //!< Absolute

	/*CPY - Compare Y with memory*/
		void CPY(AdrMode Mode);
		void CPY_$C0(); //!< Immediate
		void CPY_$C4(); //!< Zero Page 
		void CPY_$CC(); //!< Absolute 

	/*DEC - Decrement Memory by one*/
		void DEC(AdrMode Mode);
		void DEC_$C6(); //!< Zero Page 
		void DEC_$D6(); //!< Zero Page, X 
		void DEC_$CE(); //!< Absolute
		void DEC_$DE(); //!< Absolute, X 

	/*EOR (XOR) = Bitwise Exclusive Or, A^M*/
		void EOR(AdrMode Mode);
		void EOR_$49(); //!<Immediate 
		void EOR_$45(); //!< Zero Page
		void EOR_$55(); //!< Zero Page, X 
		void EOR_$4D(); //!< Absolute 
		void EOR_$5D(); //!< Absolute, X 
		void EOR_$59(); //!< Absolute, Y 
		void EOR_$41(); //Indexed Indirect X 
		void EOR_$51(); //Indirect Indexed Y

	/*Increment Memory by one*/
		void INC(AdrMode Mode);
		void INC_$E6(); //!< Zero Page 
		void INC_$F6(); //!< Zero Page, X  
		void INC_$EE(); //!< Absolute 
		void INC_$FE(); //!< Absolute, X 

	/*JMP - Goto address*/
		void JMP(AdrMode Mode);
		void JMP_$4C(); //!< Absolute 
		void JMP_$6C(); //!< Indirect 

	/*JSR Jump to SubRoutine */
		void JSR_$20(); //!< Absolute 

	/*LDA - Load Memory Into Accumulator*/
		void LDA();
		void LDA_$A9(); //!<Immediate 
		void LDA_$A5(); //!< Zero Page
		void LDA_$B5(); //!< Zero Page, X 
		void LDA_$AD(); //!< Absolute 
		void LDA_$BD(); //!< Absolute, X 
		void LDA_$B9(); //!< Absolute, Y 
		void LDA_$A1(); //Indexed Indirect X 
		void LDA_$B1(); //Indirect Indexed Y

	/*LDX - Load Memory into X Reg*/
		void LDX();
		void LDX_$A2(); //!< Immediate
		void LDX_$A6(); //!< Zero Page
		void LDX_$B6(); //!< Zero Page, Y 
		void LDX_$AE(); //!< Absolute 
		void LDX_$BE(); //!< Absolute, Y 

	/*LDY - Load Memory into Y*/
		void LDY();
		void LDY_$A0(); //!< Immediate
		void LDY_$A4(); //!< Zero Page
		void LDY_$B4(); //!< Zero Page, X
		void LDY_$AC(); //!< Absolute 
		void LDY_$BC(); //!< Absolute, X

	/*LSR - Shift right*/
		void LSR();
		void LSR_$4A(); //!< Accumulator
		void LSR_$46(); //!< Zero Page
		void LSR_$56(); //!< Zero Page, X 
		void LSR_$4E(); //!< Absolute 
		void LSR_$5E(); //!< Absolute, X 

	/*ORA - Bitwise OR, A | M*/
		void ORA(AdrMode Mode);
		void ORA_$09(); //!<Immediate 
		void ORA_$05(); //!< Zero Page
		void ORA_$15(); //!< Zero Page, X 
		void ORA_$0D(); //!< Absolute 
		void ORA_$1D(); //!< Absolute, X 
		void ORA_$19(); //!< Absolute, Y 
		void ORA_$01(); //Indexed Indirect X 
		void ORA_$11(); //Indirect Indexed Y

	/*ROL - Rotate Left*/
		void ROL(AdrMode Mode);
		void ROL_$2A(); //!< Accumulator
		void ROL_$26(); //!< Zero Page
		void ROL_$36(); //!< Zero Page, X 
		void ROL_$2E(); //!< Absolute 
		void ROL_$3E(); //!< Absolute, X 

	/*ROR - Rotate Right*/
		void ROR(AdrMode Mode);
		void ROR_$6A(); //!< Accumulator
		void ROR_$66(); //!< Zero Page
		void ROR_$76(); //!< Zero Page, X 
		void ROR_$6E(); //!< Absolute 
		void ROR_$7E(); //!< Absolute, X 

	/*SBC - Subtract Memory from Accumulator */
		void SBC(AdrMode Mode);
		void SBC_$E9(); //!<Immediate 
		void SBC_$E5(); //!< Zero Page
		void SBC_$F5(); //!< Zero Page, X 
		void SBC_$ED(); //!< Absolute 
		void SBC_$FD(); //!< Absolute, X 
		void SBC_$F9(); //!< Absolute, Y 
		void SBC_$E1(); //Indexed Indirect X 
		void SBC_$F1(); //Indirect Indexed Y

	/*STA - Store Accumulator in Memory*/
		void STA(AdrMode Mode);
		void STA_$85(); //!< Zero Page
		void STA_$95(); //!< Zero Page, X 
		void STA_$8D(); //!< Absolute 
		void STA_$9D(); //!< Absolute, X 
		void STA_$99(); //!< Absolute, Y 
		void STA_$81(); //Indexed Indirect X 
		void STA_$91(); //Indirect Indexed Y

	/*STX - Store X reg in memory*/
		void STX();
		void STX_$86(); //!< Zero Page 
		void STX_$96(); //!< Zero Page, Y
		void STX_$8E(); //!< Absolute 

	/*STY - Store Y reg in memory*/
		void STY();
		void STY_$84(); //!< Zero Page 
		void STY_$94(); //!< Zero Page, X
		void STY_$8C(); //!< Absolute 

	/*Relative addressing mode instructions*/
		//!<Branch If Carry is clear 
		void BCC_$90();

		//!< Branch if Carry is set 
		void BCS_$B0();

		//!< Branch if ZERO is set 
		void BEQ_$F0();

		//!< Branch if negative is set 
		void BMI_$30();

		//!< Branch if zero is clear 
		void BNE_D0();

		//!< Branch if negative clear 
		void BPL_$10();

		//!< Branch if overflow is clear 
		void BVC_$50();

		//!< Branch if overflow is set 
		void BVS_$70();

		//!<



#pragma endregion 151 instructions/ operation codes used by the 6502 

//** Helper Functions **//
		//!< Increments/ Decrements stack pointer by 1 to ensure wrap 
		void IncrementSP();
		void DecrementSP();

		/*	Read Memory Address - For Address Mode - helper
		*	Increments the Program Counter
		*	Establishes the address in memory
		*	Sets the (custom) M - Memory register accordingly
		*	returns the memory address
		*/
		uint16 ReadMemoryAddress(AdrMode Mode);

		uint16 GetMemZeroPage();
		uint16 GetMemZeroPageX();
		uint16 GetMemZeroPageY();
		uint16 GetMemAbsolute();
		uint16 GetMemAbsoluteX();
		uint16 GetMemAbsoluteY();
		uint16 GetMemAbsoluteIndirect();
		uint16 GetMemImmediate();
		uint16 GetMemRelative();
		uint16 GetMemIndexedIndirect();
		uint16 GetMemIndirectIndexed();

		//!< The CPUS memory/ ram 64KB total Address range from $0 - $FFFF
		UByte* RAM;

		//!< Program Counter - Holds the address of the next instruction
		//!< split into first 8 bits PCL, last 8 bits PCH 
		uint16 PC;

		//!< StackPointer - 8 Bit register - serves as offset from $0100 - No overflow - wraps $00 - $FF 
		//!<			  - Data pushed onto the stack will be placed at $0100 + SP 
		uint8 SP;

		//!< Accumerlator - 8 bit register for storing the results of arithmetric & logic operations 
		int8 Accumulator;

		//!< Index register X - 8 bit register - counter or offset for perticular addressing modes
		//!< Can be set to value recieved from memory, can be used to get or set value of stack pointer
		uint8 XReg;

		//!< Index register Y - same as Index register X with the retriction of not being able to set the stack pointer
		uint8 YReg;

		//!< Processor Status - Contains a number of bit flags in regards to the processors status (See PFLAGS)
		//1< Bit 5 should always be set to 1 
		uint8 ProcessorStatus;
	};
}

#endif 