#include "Instruction.h"
#include "CPU.h"

namespace ControlDeck
{
	Instruction::Instruction(String Name, std::function<void(AdrMode)> Operation) : 
		m_name(Name),
		m_operation(Operation)
	{
	}

	void Instruction::Execute(uint8 opCode)
	{
		const InstructionInfo* info = GetInstructionInfo(opCode);

		if (m_operation != nullptr)
		{
			m_operation(info->Mode);
			m_cpu->m_cycleCounter += info->Cycles;

			uint8 opCode = m_cpu->ReadMemory8(m_cpu->PC);
			uint8 byte1 = m_cpu->ReadMemory8(m_cpu->PC + 1);
			uint8 byte2 = m_cpu->ReadMemory8(m_cpu->PC + 2);

			if (info->Bytes == 1)
			{
				printf("%04X %02X       %s\t\t", m_cpu->PC, opCode, m_name.c_str());
			}
			else if (info->Bytes == 2)
			{
				printf("%04X %02X %02X    %s\t\t", m_cpu->PC, opCode, byte1, m_name.c_str());
			}
			else
			{
				printf("%04X %02X %02X %02X %s\t\t", m_cpu->PC, opCode, byte1, byte2, m_name.c_str());
			}

			printf("A:%02X X:%02X Y:%02X P:%02X SP:%02X\n", m_cpu->Accumulator, m_cpu->XReg, m_cpu->YReg, m_cpu->ProcessorStatus, m_cpu->SP);
		}
		else
		{
			throw("Not implemented");
		}
	}

	void Instruction::Bind(std::function<void(AdrMode)> Operation)
	{
		m_operation = Operation;
	}

	void Instruction::AddOperation(uint8 opCode, uint8 bytes, uint8 cycles, uint8 cylesPageBoundary, AdrMode mode)
	{
		m_instructions.push_back(std::move(std::make_unique<InstructionInfo>(opCode, bytes, cycles, cylesPageBoundary, mode)));
	}

	std::vector<uint8> Instruction::GetOpCodes() const
	{
		std::vector<uint8> opcodes;

		for (auto const& info : m_instructions)
		{
			opcodes.push_back(info->OpCode);
		}

		return opcodes;
	}

	const InstructionInfo* Instruction::GetInstructionInfo(uint8 OpCode) const
	{
		for (auto const& info : m_instructions)
		{
			if (info->OpCode == OpCode)
			{
				return info.get();
			}
		}

		throw("Invalid opcode for instruction.");
		return nullptr;
	}
}
