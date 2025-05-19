#pragma once

class AudioVisualizer;

#include <SDL3/SDL_audio.h>
#include "AudioData.h"

class AudioPlayer
{
	// Specification of the output format is allways the same
	const SDL_AudioSpec m_spec = {
		.format = SDL_AUDIO_F32,
		.channels = 2,
		.freq = 44200
	};

	SDL_AudioDeviceID m_device_id = 0;
	SDL_AudioStream *m_playback_stream = nullptr;

	AudioData &m_audio_data; // TODO Change to shared pointer
	bool m_playing;

	int m_minimum_audio; // Minimum number of samples in the que at any given point in bytes

public:
	explicit AudioPlayer(AudioData &audioData);

	void togglePlayback();
	void feedSamples();
	void print() {
		std::cout << std::format("Device ID:\t{}\n", m_device_id); // Doesn't have a device name fsr
		std::cout << std::format("Channels:\t{}\n", m_spec.channels);
		std::cout << std::format("Frequency:\t{}\n", m_spec.freq);
		std::cout << std::endl;
	}
	~AudioPlayer();
};
