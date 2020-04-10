#pragma once

#include "Common.h"

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
enum PFlags : uint8 {
	/*  CARRY FLAG
		The carry flag is set if there has been an 'overflow/ Underflow' from bit 7 or bit 0
		during previous operations. The carry flag can be set/ unset by calling instructions
		SEC (Set Carry Flag) And CLC (Clear Carry Flag)
	*/
	CARRY = 0x1,
	/*	ZERO FLAG
		The Zero flag is set if the preious instructions resulted in zero (0)
	*/
	ZERO = 0x2,
	/*	INTERRUPT DISABLED FLAG
		If the intterupt disabled flag is set it'll prevent the processor from responding to
		interrupt requests IRQ. Instructions: SEI (Set Intterupt Disabled) and
		CLI(Clear Interrupt Disabled)
	*/
	INTERRUPT_DISABLED = 0x4,
	/*
		Switches to BCD -- Only for 6502 - Flag ignored on 2A03/07 processors
	*/
	DECIMAL_MODE = 0x8,
	/*	BREAK COMMAND FLAG
		Signals that a break command has been called - Causing an intterupt request
	*/
	BRK_CMD = 0x10,
	/*	OVERFLOW FLAG
		Flag is set if and invalid 2 compliments number was the result of previous operations
	*/
	OVER_FLOW = 0x40,
	/*	NEGATIVE FLAG
		Set if the sign bit is 1 i.e. negative
	*/
	NEGATIVE = 0x80
};