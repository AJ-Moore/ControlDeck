
//!< Author Allan Moore 20/ 03/ 2015 

#include "CPU.h"



namespace ControlDeck{

	CPU::CPU(){

		//!< The 6502 uses a 16 bit address bus $0 - $FFFF ($FFFF+1 possible addresses)
		this->cache = new UByte[0xFFFF+1];
	}

	CPU::~CPU(){

	}

	void CPU::initialise(){

	}
}