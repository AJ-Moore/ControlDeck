#pragma once

#include "Common.h"
#include <SDL2/SDL_audio.h>

namespace ControlDeck
{
	enum WaveformType
	{
		PULSE = 0x1,
		TRIANGLE = 0x2
	};

	class WaveformGenerator
	{
	private:
		const int m_samples = 256;
		
		int m_waveOffset = 0;
		int m_hertz = 44;
		float m_duty = 0;
		float m_volume = 15;
		bool m_muted = true;
		std::vector<Uint8> m_waveData;
		SDL_AudioSpec m_audioSpec;

		/* APU Length counter, duration of waveform
		* see https://www.nesdev.org/wiki/APU_Length_Counter
		* When clocked by frame counter length counter decremented accept when:
		* - Lenght counter is 0
		* - The halt flag is set (see APU Flags)
		*/
		int m_lengthCounter = 0;
		WaveformType m_waveformType = WaveformType::PULSE;

		static void AudioCallback(void* userdata, Uint8* stream, int len);
	public:
		void Init(WaveformType type);
		void SetHertz(float value);
		void SetVolume(float value) { m_volume = value; }
		void SetDuty(float value) { m_duty = value; }
		void SetMute(bool value) { m_muted = value; }
	};
}
