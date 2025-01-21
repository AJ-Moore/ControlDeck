#pragma once
#include "Types.h"
#include "WaveformGenerator.h"
#include "CPU.h"

namespace ControlDeck
{
	/* APU - Audio Processing Unit
	* 
	*/
	class APU
	{
	public:
		APU() = delete; 
		APU(CPU* cpu);

		void Init();
		void Update();

		/*APU Registers
		* see https://www.nesdev.org/wiki/APU#Pulse_($4000-4007)	
		* 
		* Pulse 1 $4000 - $4003
		* Pulse 2 $4004 - $4007
		* Triangle 4008 - $400B
		* Noise $400C–$400F
		* DMC $4010–$4013
		* $4015 (All channels) channel enable and length counter status.
		* $4017 (All channels) frame counter.
		*/

		/* Pulse 1 Channel(write) $4000 - $4003
		* $4000/ $4004 - Duty & envelope/volume (write) DDLC VVVV
		* Duty (D), envelope loop / length counter halt (L), constant volume (C), volume/envelope (V)*/
		static const uint16 PULSE1_DUTY_ENVELOPE_WRITE = 0x4000;
		static const uint16 PULSE1_SWEEP = 0x4001;
		static const uint16 PULSE1_TIMER_LOW = 0x4002;
		static const uint16 PULSE1_COUNTER_TIMER_HIGH = 0x4003;

		static const uint16 PULSE2_TIMER_LOW = 0x4006;
		static const uint16 PULSE2_COUNTER_TIMER_HIGH = 0x4007;

		static const uint16 PULSE2_DUTY_ENVELOPE_WRITE = 0x4004;

		// Triangle 
		static const uint16 APU_TRIANGLE_COUNTER = 0x4008;
		static const uint16 APU_TRIANGLE_TIMER = 0x400A;
		static const uint16 APU_TRIANGLE_COUNTER_RELOAD_TIMER_HIGH = 0x400B;

		const uint8 DUTY_CYCLE_MASK = 0xC0;
		const uint8 SHIFT_COUNT_MASK = 0x7;
		const uint8 NEGATE_MASK = 0x8;
		const uint8 VOLUME_MASK = 0x15;
		
	private:
		CPU* m_cpu;
		// Memory mapped APU memory/ registers.
		std::vector<uint8> m_apuMemory;

		WaveformGenerator m_triangleWave;
		WaveformGenerator m_pulse1;
		WaveformGenerator m_pulse2;
	};
}