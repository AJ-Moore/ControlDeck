#include "WaveformGenerator.h"

void ControlDeck::WaveformGenerator::Init(WaveformType type)
{
	m_waveformType = type;
	m_waveData.resize(m_samples);

	SDL_zero(m_audioSpec);
	m_audioSpec.freq = 44100;
	m_audioSpec.format = AUDIO_U8;
	m_audioSpec.channels = 1;
	m_audioSpec.silence = 0;
	m_audioSpec.samples = m_samples;
	m_audioSpec.padding = 0;
	m_audioSpec.size = 0;
	m_audioSpec.userdata = this;

	m_audioSpec.callback = AudioCallback;
	if (SDL_OpenAudio(&m_audioSpec, NULL) < 0)
	{
		printf("Unable to open audio devicve [%s]\n", SDL_GetError());
		// ignore and continue
		//exit(-1);
	}
	SDL_PauseAudio(0);
}

void ControlDeck::WaveformGenerator::SetHertz(float value)
{
	m_hertz = value;
}


void ControlDeck::WaveformGenerator::AudioCallback(void* userdata, Uint8* stream, int len)
{
	WaveformGenerator*  ptr = (WaveformGenerator*)userdata;

	if (ptr->m_muted)
	{
		return;
	}

	float amplitude = 1.0f; // 0-1
	float sampleRate = 44100;
	float sampleInSeconds = ptr->m_samples/ sampleRate;
	float samplePerCPU = sampleRate / 1789773.0f;
	float frequency = ((2 * PI * ptr->m_hertz)/ sampleRate);

	// Sine
	//for (int i = 0; i < len; i++)
	//{
	//	ptr->m_waveOffset++;
	//	ptr->m_waveData[i] = (Uint8)((amplitude * 127.5f * sin(frequency * (double)ptr->m_waveOffset)) + 127.5);
	//}

	double halfPi = PI * 0.5f;
	
	if (ptr->m_waveformType == WaveformType::TRIANGLE)
	{
		// Triangle
		for (int i = 0; i < len; i++)
		{
			float freq = frequency * (double)ptr->m_waveOffset;
			float offstep = floor(freq / halfPi);
			float t = (freq - offstep);

			ptr->m_waveOffset++;
			ptr->m_waveData[i] = t * 255.0f;
			//stream[i] = ptr->m_waveData[i];
		}
	}
	else if (ptr->m_waveformType == WaveformType::PULSE)
	{
		static int pulseCounter = 0;
		float pulsePeriod = (1790000/2) / ((ptr->m_hertz + 1) * 8);
		pulsePeriod -= 1;

		float duty = 0.125;

		if (ptr->m_duty > 0)
		{
			duty = ptr->m_duty / 4.0f; 
		}
		
		float highTime = pulsePeriod * duty;

		// Fill the stream with the pulse wave
		for (int i = 0; i < len; i++) {
			// Generate the pulse wave (alternating between 0 and 255)
			if (pulseCounter < highTime) {
				ptr->m_waveData[i] = 255;  // On (max value for 8-bit)
				stream[i] = (ptr->m_volume/15) * 255;  // On (max value for 8-bit)
			}
			else {
				ptr->m_waveData[i] = 0;    // Off (min value for 8-bit)
				stream[i] = 0;    // Off (min value for 8-bit)
			}

			pulseCounter++;
			if (pulseCounter >= pulsePeriod) {
				pulseCounter = 0;  // Reset to create a new cycle
			}
		}
	}

	//SDL_MixAudio(stream, &ptr->m_waveData[0], len, ptr->m_muted ? 0 : SDL_MIX_MAXVOLUME);
}
