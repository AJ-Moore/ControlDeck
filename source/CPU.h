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

#include <iostream>
#include "Types.h"

namespace ControlDeck{

	class CPU{
	public: 
		CPU();
		~CPU(); 

		void initialise();

		//!< Fetch next instruction and execute 
		void update(); 

		U8 readMemory8(U16 Addr);
		//U16 readMemory16(U16 Addr); 

		//!< Used for writing U8 to perticular memory address 
		//!< We use a writeMemory function in order to ensure that data is
		//!< mirrored correctly... basically 
		void writeMemory8(U16 Addr, U8 Data); 
		//!< Haven't decide on whether necessary to have an additional function 
		//!< for writing memory in case where operand makes use of 16 bits of 
		//!< data... considering most data is actually handled in 8 bits.. meh
		//!< I'll decide as and when necessary!
		//void writeMemory16(U16 Addr, U16 Data);

/** Useful Constants **/
		//!< The location from which the stack begins 
		const U16 STACK			=				0x0100; 

		//!< Location of PRG ROM Upper Bank 
		const U16 PRGROM_UPPER	=				0xC000;

		//!< Location of PRG ROM Lower Bank
		const U16 PRGROM_LOWER	=				0x8000; 

	private:

		//!< Insert about 151 instructions here D:
#pragma region _6502_Instructions 

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


#pragma endregion 151 instructions/ operation codes used by the 6502 

//** Helper Functions **//
		//!< Increments/ Decrements stack pointer by 1 to ensure wrap 
		void incrementSP();
		void decrementSP(); 

		//!< The CPUS memory/ ram 64KB total Address range from $0 - $FFFF
		UByte* RAM; 

		//!< Program Counter - Holds the address of the next instruction
		//!< split into first 8 bits PCL, last 8 bits PCH 
		U16 PC; 

		//!< StackPointer - 8 Bit register - serves as offset from $0100 - No overflow - wraps $00 - $FF 
		//!<			  - Data pushed onto the stack will be placed at $0100 + SP 
		U8 SP; 

		//!< Accumerlator - 8 bit register for storing the results of arithmetric & logic operations 
		S8 A;

		//!< Index register X - 8 bit register - counter or offset for perticular addressing modes
		//!< Can be set to value recieved from memory, can be used to get or set value of stack pointer
		U8 X;

		//!< Index register Y - same as Index register X with the retriction of not being able to set the stack pointer
		U8 Y; 

		//!< Processor Status - Contains a number of bit flags in regards to the processors status (See PFLAGS)
		//1< Bit 5 should always be set to 1 
		U8 P; 

	};

	/***Processor Status Bits************ 
		bit 0 | C = Carry
				Z = Zero 
				I = Interrupt Disabled 
				D = Decimal Mode 
				B = Break Command Flag 
				----------------------
				V = Overflow Flag 
		bit 7 | S = Sign flag 
	*************************************/

	//!< Processor Status Flags
	enum PFLAGS : U8{
		/*  CARRY FLAG 
			The carry flag is set if there has been an 'overflow/ Underflow' from bit 7 or bit 0 
			during previous operations. The carry flag can be set/ unset by calling instructions
			SEC (Set Carry Flag) And CLC (Clear Carry Flag)
		*/
		CARRY					= 0x1,
		/*	ZERO FLAG 
			The Zero flag is set if the preious instructions resulted in zero (0)  
		*/
		ZERO					= 0x2,
		/*	INTERRUPT DISABLED FLAG 
			If the intterupt disabled flag is set it'll prevent the processor from responding to 
			interrupt requests IRQ. Instructions: SEI (Set Intterupt Disabled) and 
			CLI(Clear Interrupt Disabled)	
		*/
		INTERRUPT_DISABLED		= 0x4,
		/*
			Switches to BCD -- Only for 6502 - Flag ignored on 2A03/07 processors 
		*/
		DECIMAL_MODE			= 0x8,
		/*	BREAK COMMAND FLAG 
			Signals that a break command has been called - Causing an intterupt request 
		*/
		BRK_CMD					= 0x10,
		/*	OVERFLOW FLAG 
			Flag is set if and invalid 2 compliments number was the result of previous operations 
		*/
		OVER_FLOW				= 0x40,
		/*	NEGATIVE FLAG
			Set if the sign bit is 1 i.e. negative
		*/
		NEGATIVE				= 0x80
	};

}

#endif 