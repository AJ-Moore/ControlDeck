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
#include "Cartridge.h"

namespace ControlDeck
{
	class CPU
	{
	public:
		CPU();
		void Initialise();
		void Update();

		void LoadCartridge(Cartridge* cartridge);

		// Read/ Write bytes to memory
		uint8 ReadMemory8(uint16 Addr);
		uint16 ReadMemory16(uint16 Addr);
		void WriteMemory8(uint16 Addr, uint8 Data);

	private:
		/** Useful Constants - STACK(0x0100), PRGROM_UPPER(0xC000),PRGROM_LOWER(0x8000) **/
		//!< The locations from which the stack, PRG ROM UPPER/ LOWER BANKS begin
		const uint16 STACK = 0x0100;
		const uint16 PRGROM_UPPER = 0xC000;
		const uint16 PRGROM_LOWER = 0x8000;

		// CPU ADDRESS LOCATIONS
		// https://wiki.nesdev.com/w/index.php/PPU_registers#OAMADDR
		static const uint16 PPU_CTRL_ADR = 0x2000;
		static const uint16 PPU_MASK_ADR = 0x2001;
		static const uint16 PPU_STATUS_ADR = 0x2002;
		static const uint16 OAM_ADR_ADR = 0x2003;
		static const uint16 OAM_DATA_ADR = 0x2004;
		static const uint16 PPU_SCROLL_ADR = 0x2005;
		static const uint16 PPU_ADR = 0x2006;
		static const uint16 PPU_DATA_ADR = 0x2007;
		static const uint16 OAM_DMA_ADR = 0x4014;

		void SetProcessorFlag(uint8 Flag, bool bEnabled);
		void PushStack8(uint8 memory);
		void PushStack16(uint16 memory);
		uint8 PopStack8();
		uint16 PopStack16();
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

		Cartridge* m_loadedCartridge = nullptr;
		uint32 m_cycleCounter = 0;

		//!< RAM - The CPUS memory/ ram 64KB total Address range from $0 - $FFFF
		std::vector<ubyte> RAM;
		
		//!< PC - Program Counter - Holds the address of the next instruction, split into first 8 bits PCL, last 8 bits PCH
		uint16 PC = PRGROM_UPPER;

		//!< StackPointer - 8 Bit register - serves as offset from $0100 - No overflow - wraps $00 - $FF 
		//!<			  - Data pushed onto the stack will be placed at $0100 + SP 
		uint8 SP = 0xFF;

		//!< Accumerlator - 8 bit register for storing the results of arithmetric & logic operations 
		int8 Accumulator = 0;

		//!< Index register X - 8 bit register - counter or offset for perticular addressing modes
		//!< Can be set to value recieved from memory, can be used to get or set value of stack pointer
		uint8 XReg = 0;

		//!< Index register Y - same as Index register X with the retriction of not being able to set the stack pointer
		uint8 YReg = 0;

		//!< Processor Status - Contains a number of bit flags in regards to the processors status (See PFLAGS)
		//!< Bit 5 should always be set to 1 
		uint8 ProcessorStatus = 0;

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

		/*BIT - Test b*/
		void BIT(AdrMode Mode);

		/*CMP = Compare Acumulator with Memory*/
		void CMP(AdrMode Mode);

		/*CPX - Comapre X with Memory*/
		void CPX(AdrMode Mode);

		/*CPY - Compare Y with memory*/
		void CPY(AdrMode Mode);

		/*DEC - Decrement Memory by one*/
		void DEC(AdrMode Mode);

		/*EOR (XOR) = Bitwise Exclusive Or, A^M*/
		void EOR(AdrMode Mode);

		/*Increment Memory by one*/
		void INC(AdrMode Mode);

		/*JMP - Goto address*/
		void JMP(AdrMode Mode);

		/*JSR Jump to SubRoutine */
		void JSR_$20(); //!< Absolute 

		/*LDA - Load Memory Into Accumulator*/
		void LDA(AdrMode Mode);

		/*LDX - Load Memory into X Reg*/
		void LDX(AdrMode Mode);

		/*LDY - Load Memory into Y*/
		void LDY(AdrMode Mode);

		/*LSR - Shift right*/
		void LSR(AdrMode Mode);

		/*ORA - Bitwise OR, A | M*/
		void ORA(AdrMode Mode);

		/*ROL - Rotate Left*/
		void ROL(AdrMode Mode);

		/*ROR - Rotate Right*/
		void ROR(AdrMode Mode);

		/*SBC - Subtract Memory from Accumulator */
		void SBC(AdrMode Mode);

		/*STA - Store Accumulator in Memory*/
		void STA(AdrMode Mode);

		/*STX - Store X reg in memory*/
		void STX(AdrMode Mode);

		/*STY - Store Y reg in memory*/
		void STY(AdrMode Mode);

#pragma region relative

		//!<Branch If Carry is clear 
		void BCC_$90(AdrMode Mode);

		//!< Branch if Carry is set 
		void BCS_$B0(AdrMode Mode);

		//!< Branch if ZERO is set 
		void BEQ_$F0(AdrMode Mode);

		//!< Branch if negative is set 
		void BMI_$30(AdrMode Mode);

		//!< Branch if zero is clear 
		void BNE_D0(AdrMode Mode);

		//!< Branch if negative clear 
		void BPL_$10(AdrMode Mode);

		//!< Branch if overflow is clear 
		void BVC_$50(AdrMode Mode);

		//!< Branch if overflow is set 
		void BVS_$70(AdrMode Mode);

#pragma endregion rel

#pragma endregion instructions/ operation codes used by the 6502 

	};
}

#endif 