#pragma once

#include "Common.h"
#include "AddressingMode.h"
#include "CPU.h"

namespace ControlDeck
{
	class InstructionInfo
	{
	public:
		InstructionInfo(uint8 opCode, uint8 bytes, uint8 cycles, uint8 cyclesPage, AdrMode mode) :
			OpCode(opCode),
			Bytes(bytes),
			Cycles(cycles),
			Mode(mode),
			CyclePageBoundary(cyclesPage)
		{
		}

		uint8 OpCode = 0;
		uint8 Bytes = 1;
		uint8 Cycles = 0;
		uint8 CyclePageBoundary = 0;
		AdrMode Mode = AdrMode::IMPLIED;
	};

	class Instruction
	{
		friend class CPU;
	public:
		Instruction(String Name, std::function<void(AdrMode)>);

		virtual void Init() {}
		virtual void Execute(uint8 opCode);

		String GetInfo() const { return m_info; }
		void SetInfo(String info) { m_info = info; }
		String GetName() const { return m_name; }

		void Bind(std::function<void(AdrMode)>);
		void AddOperation(uint8 opCode, uint8 bytes, uint8 cycles, uint8 cylesPageBoundary, AdrMode mode);
		std::vector<uint8> GetOpCodes() const;

	protected:
		CPU* m_cpu = nullptr;
		String m_name = "ERR";
		String m_info = "No Info given.";
		std::vector<UniquePtr<InstructionInfo>> m_instructions;
		std::function<void(AdrMode)> m_operation;

		const InstructionInfo* GetInstructionInfo(uint8 OpCode) const;
	};
}

