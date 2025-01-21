#include "APU.h"

ControlDeck::APU::APU(CPU* cpu)
{
	m_cpu = cpu;
}

void ControlDeck::APU::Init()
{
	//m_triangleWave.Init(WaveformType::TRIANGLE);
	m_pulse1.Init(WaveformType::PULSE);
	//m_pulse2.Init(WaveformType::PULSE);
}

void ControlDeck::APU::Update()
{
	const float apuClock = 1790000.0f;  // 1.79 MHz in Hz

	// Update triangle wave
	uint8 timerHighMask = 0x7; // Mask high bits timer high ---- -HHH 
	uint16 highByte = m_cpu->ReadMemory8(APU_TRIANGLE_COUNTER_RELOAD_TIMER_HIGH) & timerHighMask;
	highByte = highByte << 8;
	uint16 lowByte = m_cpu->ReadMemory8(APU_TRIANGLE_TIMER);
	int frequency = (0x1000 - (highByte | lowByte));

	if (frequency == 0)
	{
		return;
	}

	m_triangleWave.SetHertz(apuClock/(float)frequency);

	// Update PULSE 


	uint8 pulse1Sweep = m_cpu->ReadMemory8(PULSE1_SWEEP);
	m_pulse1.SetDuty((DUTY_CYCLE_MASK & m_cpu->ReadMemory8(PULSE1_DUTY_ENVELOPE_WRITE)) >> 6);
	int pulse1ShiftCount = m_cpu->ReadMemory8(PULSE1_SWEEP) & SHIFT_COUNT_MASK;
	bool pulse1Negate = m_cpu->ReadMemory8(PULSE1_SWEEP) & NEGATE_MASK;
	bool pulse1Volume = m_cpu->ReadMemory8(PULSE1_DUTY_ENVELOPE_WRITE) & VOLUME_MASK;

	highByte = m_cpu->ReadMemory8(PULSE1_COUNTER_TIMER_HIGH) & timerHighMask;
	highByte = highByte << 8;
	lowByte = m_cpu->ReadMemory8(PULSE1_TIMER_LOW);
	frequency = (highByte | lowByte);
	int frequencyModulation = frequency >> pulse1ShiftCount;
	frequency += pulse1Negate ? -frequencyModulation : frequencyModulation;
	//m_pulse1.SetHertz(apuClock / (float)frequency);
	m_pulse1.SetHertz(frequency);
	m_pulse1.SetVolume(pulse1Volume);

	bool sweepEnabled = (0x80 & m_cpu->ReadMemory8(PULSE1_SWEEP));

	if (pulse1ShiftCount == 0 || pulse1Volume == 0)
	{
		m_pulse1.SetMute(true);
	}
	else
	{
		m_pulse1.SetMute(false);
	}

	// Update PULSE 
	highByte = m_cpu->ReadMemory8(PULSE2_COUNTER_TIMER_HIGH) & timerHighMask;
	//highByte = 0;
	highByte = highByte << 8;
	lowByte = m_cpu->ReadMemory8(PULSE2_TIMER_LOW);
	frequency = ((highByte | lowByte));
	m_pulse2.SetHertz(apuClock / (float)frequency);

	m_pulse2.SetDuty((DUTY_CYCLE_MASK & m_cpu->ReadMemory8(PULSE2_DUTY_ENVELOPE_WRITE)) >> 6);

}
