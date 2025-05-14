#pragma once

#include <SDL3/SDL_audio.h>

class AudioPlayer
{
private:
	// Specification of the output format is allways the same
	const SDL_AudioSpec m_out_spec = {
		.format = SDL_AUDIO_F32,
		.channels = 2,
		.freq = 44200
	};

	SDL_AudioDeviceID m_device_id;
	SDL_AudioStream *m_audio_stream;

	const AudioData &m_audio_data;
	bool m_playing;
	size_t m_cursor = 0;
	// int m_minimum_audio;

public:
	AudioPlayer(const AudioData &audioData);

	void togglePlayback();
	void feedSamples();
	void print() {
		std::cout << std::format("Device Name:\t{}\n", SDL_GetAudioDeviceName(m_device_id));
	}
	~AudioPlayer();
};