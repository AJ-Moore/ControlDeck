// Copyright © Allan Moore April 2020

#pragma once

/*	Address modes enumeration, for opcode aaabbbcc. bbb bits determine addressing mode
*	000 - (Zero page, X)
*	001 - Zero page
*	010 - Immediate
*	011 - Absolute
*	100 - (Zero page), Y
*	101 - Zero page, X
*	110 - Absolute, Y
*	111 - Absolute, X
*/

namespace ControlDeck
{
	//!<
	enum class AdrMode
	{
		NONE,

		//!< Byte follow opcode is byte in first page of ram to be used RAM[N] where N is byte following opcode.
		ZERO_PAGE,

		//!< Effectivly same as above where location equals N + X Register.
		ZERO_PAGEX,

		//!< Effectivly same as above where location equals N + Y Register.
		ZERO_PAGEY,

		//!< 2 bytes following opcode specify location of byte to be used.
		ABSOLUTE,

		//!< 2 bytes following opcode + X register specify byte to be used.
		ABSOLUTEX,

		//!< 2 bytes following opcode + Y register specify byte to be used.
		ABSOLUTEY,

		//!< Absolute indirect - The two bytes following opcode specify address of 2 bytes that contain the absolute address of byte of data to use.
		ABSOLUTE_INDIRECT,

		//!< Instruction is implied, data, destination, function implied by instruction
		IMPLIED,

		//!< Targets the accumaltor (A) i.e. 'LSR A', 'ROL A' 
		ACCUMULATOR,

		//!< Data is defined in byte following the opcode 
		IMMEDIATE,

		//!< Sum of the program counter and following byte in memory is used
		RELATIVE,

		//!< Index Indirect, only used by X Register
		INDEXED_INDIRECT,

		//!< Index Indirect, only used by Y Register
		INDIRECT_INDEXED
	};
}
