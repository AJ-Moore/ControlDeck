
//!< Author Allan Moore 20/ 03/ 2015 

#include "CPU.h"



namespace ControlDeck{

	CPU::CPU():STACK(0x0100), PRGROM_UPPER(0xC000),PRGROM_LOWER(0x8000){

		//!< The 6502 uses a 16 bit address bus $0 - $FFFF ($FFFF+1 possible addresses) 
		this->RAM = new UByte[0xFFFF]; //!< err $FFFF invalid, wrap to 0?

		//!< Initialise RAM to NAUGHT
		for (U16 i = 0; i < 0xFFFF; i++){
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

	CPU::~CPU(){

	}

	void CPU::initialise(){

	}

	void CPU::update(){

		//?? Some Unknown Code Regarding CPU cycles should be thought out and put here 

		//Switch (ProgramCounter) -> Call instructions -> probably faster than using function pointers for each operation code
		while (true){//!< While something or other execute instructions
			U8 _opcode = this->readMemory8(this->PC);

			//!< Switch instructions
			switch (_opcode){
			default:
				//!< Whats the default case?
				break; 
			}

			//Increment the program counter 
			this->PC++;	
		}
	}

/** CPU MEMORY READ & WRITE **/

	void CPU::writeMemory8(U16 Addr, U8 Data){

		//!< Not sure what to do here yet -> just so I don't forget
		if (Addr == 0xFFFF){
			//throw eMAOOR;
		}

		//!< * Memory at $000-$07FF mirrored at $0800, $1000, $1800
		//!< * Assuming any data written to range $0800-$2000 should
		//!< * Mirroring will occur appriopiately 
		if (Addr < 0x2000){
			if (Addr < 0x0800){
				//!< Mirroring at $0800, $1000 and $1800 
				this->RAM[Addr] = Data;
				this->RAM[Addr + 0x0800] = Data; 
				this->RAM[Addr + 0x1000] = Data; 
				this->RAM[Addr + 0x1800] = Data; 
				return;
			}
			else if (Addr < 0x1000){
				this->RAM[Addr - 0x0800] = Data;
				this->RAM[Addr] = Data; 
				this->RAM[Addr + 0x0800] = Data; 
				this->RAM[Addr + 0x1000] = Data; 
				return;
			}
			else if (Addr < 0x1800){
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
		if (Addr < 0x4000){
			//!< Minus the starting position and get the remainder of the addr / 8 in order to establish 
			//!< the bytes that should be mirrored (mByte -> byte to be mirrored)
			U8 _mByte = (Addr - 0x2000) % 8 ;

			//!< For every byte in range $2000 - $3FFF (kinda) 
			for (U16 _m = _mByte; _m < 0x4000; _m+= 8){
				this->RAM[0x2000 + _m] = Data; 
			}

		}




	}

	U8 CPU::readMemory8(U16 Addr){
		return this->RAM[Addr];
	}

/** ======================= **/

//#pragma region _6502_Instructions 

/***Implied Addressing Mode Instructions***/
	void CPU::BRK_$00(){
		this->PC++;
		writeMemory8(this->STACK + this->SP, PC>>8); 
		this->decrementSP(); 
		writeMemory8(this->STACK + this->SP, PC);	
		this->decrementSP();

	}
	void CPU::CLC_$18(){
		this->P &= ~(PFLAGS::CARRY);
	}
	void CPU::CLD_$D8(){
		this->P &= ~(PFLAGS::DECIMAL_MODE);
	}
	void CPU::CLI_$58(){
		this->P &= ~(PFLAGS::INTERRUPT_DISABLED);
	}
	void CPU::CLV_$B8(){
		this->P &= ~(PFLAGS::OVER_FLOW); 
	}
	void CPU::DEX_$CA(){
		this->X--; 

		// If result of decriment is zero set zero flag 
		(this->X == 0) ? this->PC |= PFLAGS::ZERO : this->PC &= ~PFLAGS::ZERO;
		
		// If bit 7 is set, set negative flag 
		(this->X & (1<<7)) ? this->PC |= PFLAGS::NEGATIVE : this->PC &= ~PFLAGS::NEGATIVE;
	}
	void CPU::DEY_$88(){
		this->Y--; 

		// If result of decriment is zero set zero flag 
		(this->Y == 0) ? this->PC |= PFLAGS::ZERO : this->PC &= ~PFLAGS::ZERO;
		
		// If bit 7 is set, set negative flag 
		(this->Y & (1<<7)) ? this->PC |= PFLAGS::NEGATIVE : this->PC &= ~PFLAGS::NEGATIVE;
	}
	void CPU::INX_$E8(){
		this->X++; 

		// If result of decriment is zero set zero flag 
		(this->X == 0) ? this->PC |= PFLAGS::ZERO : this->PC &= ~PFLAGS::ZERO;
		
		// If bit 7 is set, set negative flag 
		(this->X & (1<<7)) ? this->PC |= PFLAGS::NEGATIVE : this->PC &= ~PFLAGS::NEGATIVE;
	}
	void CPU::INY_$C8(){
		this->Y++; 

		// If result of decriment is zero set zero flag 
		(this->Y == 0) ? this->PC |= PFLAGS::ZERO : this->PC &= ~PFLAGS::ZERO;
		
		// If bit 7 is set, set negative flag 
		(this->Y & (1<<7)) ? this->PC |= PFLAGS::NEGATIVE : this->PC &= ~PFLAGS::NEGATIVE;
	}
	void CPU::NOP_$EA(){
		//!< Does nothing 
	}
	void CPU::PHA_$48(){
		writeMemory8(this->STACK + this->SP, this->A); 
		this->decrementSP();
	}
	void CPU::PHP_$08(){
		writeMemory8(this->STACK + this->SP, this->P);
		this->decrementSP();
	}
	void CPU::PLA_$68(){
		this->incrementSP(); 
		this->A = readMemory8(this->STACK + this->SP);

		(this->A == 0) ? this->P |= PFLAGS::ZERO : this->P &= ~PFLAGS::ZERO;
		(this->A & (1<<7)) ? this->P |= PFLAGS::NEGATIVE : this->P &= ~PFLAGS::NEGATIVE;
	}
	void CPU::PLP_$28(){
		this->incrementSP(); 
		this->P = readMemory8(this->STACK + this->SP); 
	}
	void CPU::RTI_$40(){
		this->incrementSP(); 
		this->P = readMemory8(this->STACK + this->SP);

		//!< Get/ restore? the instructions address from memory ??
		//!< Needs Checking
		this->incrementSP();
		U16 _l = readMemory8(this->STACK + this->SP);
		this->incrementSP();
		U16 _h = readMemory8(this->STACK + this->SP)<<8;

		this->PC = _h | _l ;
		
	}
	void CPU::RTS_$60(){
		this->incrementSP();
		U16 _l = readMemory8(this->STACK + this->SP);
		this->incrementSP();
		U16 _h = readMemory8(this->STACK + this->SP)<<8; 
		this->PC = _h | _l;
	}
	void CPU::SEC_$38(){
		this->P |= PFLAGS::CARRY;
	}
	void CPU::SED_$f8(){
		this->P |= PFLAGS::DECIMAL_MODE;
	}
	void CPU::SEI_$78(){
		this->P |= PFLAGS::INTERRUPT_DISABLED;
	}
	void CPU::TAX_$AA(){
		this->X = this->A;
		(this->X == 0) ? this->P |= PFLAGS::ZERO : this->P &= ~PFLAGS::ZERO;
		(this->X & (1<<7)) ? this->P |= PFLAGS::NEGATIVE : this->P &= ~ PFLAGS::NEGATIVE;

	}
	void CPU::TAY_$A8(){
		this->Y = this->A;
		(this->Y == 0) ? this->P |= PFLAGS::ZERO : this->P &= ~PFLAGS::ZERO;
		(this->Y & (1<<7)) ? this->P |= PFLAGS::NEGATIVE : this->P &= ~ PFLAGS::NEGATIVE;
	}
	void CPU::TSX_$BA(){
		this->X = this->SP;
		(this->X == 0) ? this->P |= PFLAGS::ZERO : this->P &= ~PFLAGS::ZERO;
		(this->X & (1<<7)) ? this->P |= PFLAGS::NEGATIVE : this->P &= ~ PFLAGS::NEGATIVE;
	}
	void CPU::TXA_$8A(){
		this->A = this->X;
		(this->A == 0) ? this->P |= PFLAGS::ZERO : this->P &= ~PFLAGS::ZERO;
		(this->A & (1<<7)) ? this->P |= PFLAGS::NEGATIVE : this->P &= ~ PFLAGS::NEGATIVE;
	}
	void CPU::TXS_$9A(){
		this->SP = this->X;
	}
	void CPU::TYA_$98(){
		this->A = this->Y;
		(this->A == 0) ? this->P |= PFLAGS::ZERO : this->P &= ~PFLAGS::ZERO;
		(this->A & (1<<7)) ? this->P |= PFLAGS::NEGATIVE : this->P &= ~ PFLAGS::NEGATIVE;
	}

/**Add memory to accumulator**/
	/*Add memory to Accumulator with carry*/
	void CPU::ADC(U16 _M){

		//!< Clear the carry flag 
		//this->CLC_$18(); I'm not sure why this made sense to me

		//!< Result 
		U16 _R = (this->P & PFLAGS::CARRY) + this->A + _M;	

		//!< Makes sence obviously presuming _M unsigned
		//!< I hope this is right D:
		((this->A & PFLAGS::NEGATIVE) & (_R & PFLAGS::NEGATIVE))
			? P |= PFLAGS::OVER_FLOW : P & ~(PFLAGS::OVER_FLOW) ;

		//!< So basically if zero
		(_R == 0) ? P |= PFLAGS::ZERO : 
					P & ~(PFLAGS::ZERO);

		//!< check whether to set carry 
		(_R > 255) ? this->P |= PFLAGS::CARRY :
					 this->P &~ PFLAGS::CARRY;

		//!< Set Accumulator 
		A = _R;
	}
	void CPU::ADC_$69(){
		//!< Immediate 
		this->PC++; 
		U16 _M = this->readMemory8(this->PC);

		this->ADC(_M);

		this->PC++;

	}
	void CPU::ADC_$65(){//!<Zero page 
		this->PC++; 
		U16 _M = this->readMemory8(this->readMemory8(this->PC));
		this->ADC(_M);
		this->PC++;
	}
	void CPU::ADC_$75(){//!< Indexed Zero Page, X	
		this->PC++; 
		U16 _Adr = this->readMemory8(this->PC);
		//!< wrap
		_Adr = (_Adr > 255) ? _Adr-255 : _Adr;
				   
		U16 _M = this->readMemory8(_Adr);
		this->ADC(_M);
		this->PC++;
	} 
	void CPU::ADC_$6D(){//!< Absolute (16 bit adr)
		this->PC++; 
		U16 _Adr = this->readMemory8(this->PC); 
		_Adr &= (this->readMemory8(this->PC+1) << 8); 

		U16 _M = this->readMemory8(_Adr); 
		this->ADC(_M);
		this->PC += 2;
	}
	void CPU::ADC_$7D(){ //!< Indexed Absolute X (16 bit adr)
		this->PC++; 
		U16 _Adr = this->readMemory8(this->PC); 
		_Adr &= (this->readMemory8(this->PC+1) << 8); 
		_Adr += this->X;

		U16 _M = this->readMemory8(_Adr); 
		this->ADC(_M);
		this->PC += 2;
	}
	void CPU::ADC_$79(){ //!< Indexed Absolute Y (16 bit adr)
		this->PC++; 
		U16 _Adr = this->readMemory8(this->PC); 
		_Adr &= (this->readMemory8(this->PC+1) << 8); 
		_Adr += this->Y;

		U16 _M = this->readMemory8(_Adr); 
		this->ADC(_M);
		this->PC += 2;
	}
	void CPU::ADC_$61(){ //!< Indexed Indirect X 
		this->PC++; 
		U16 _Adr = this->readMemory8(this->PC); 
		_Adr &= (this->readMemory8(this->PC+1) << 8); 
		_Adr += this->X;

		U16 _AdrIndirect = this->readMemory8(_Adr); 
		_AdrIndirect &= (this->readMemory8(_Adr+1) << 8); 

		U16 _M = this->readMemory8(_AdrIndirect); 
		this->ADC(_M);
		this->PC += 2;
	}
	void CPU::ADC_$71(){ //!< Indirect Indixed Y
		this->PC++; 
		U16 _Adr = this->readMemory8(this->PC); 

		U16 _AdrIndirect = this->readMemory8(_Adr); 
		_AdrIndirect &= (this->readMemory8(_Adr+1) << 8); 

		U16 _M = this->readMemory8(_AdrIndirect + this->Y); 
		this->ADC(_M);
		this->PC++;
	}

//#pragma endregion 151 instructions/ operation codes used by the 6502 

// Helper Functions 
	void CPU::incrementSP(){
		(this->SP == 0xFF) ? this->SP = 0 : this->SP++; 
	}

	void CPU::decrementSP(){
		(this->SP == 0) ? this->SP = 0xFF : this->SP--; 
	}

	//!< Establishes correct Addr location, 
	U8 CPU::setAddrMode(U8 AddrMode){

		switch (AddrMode){
		case ADR_M::ZERO_PAGE:
			break; 
		case ADR_M::ZERO_PAGEX:
			break; 
		case ADR_M::ZERO_PAGEY:
			break;
		case ADR_M::ABSOLUTE:
			break; 
		case ADR_M::ABSOLUTEY:
			break; 
		case ADR_M::ABSOLUTEX:
			break; 
		case ADR_M::INDIRECT:
			break; 
		case ADR_M::IMPLIED:
			break; 
		case ADR_M::ACCUMULATOR:
			break; 
		case ADR_M::IMMEDIATE:
			break; 
		case ADR_M::RELATIVE:
			break; 
		case ADR_M::INDEXED_INDIRECT:
			break; 
		case ADR_M::INDIRECT_INDEXED:
			break; 
		default:
			throw ("Invalid Address Mode");
		}


	}

}