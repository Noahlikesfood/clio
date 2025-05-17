#pragma once

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
	SDL_AudioStream *m_output_stream = nullptr;

	const AudioData &m_audio_data; // TODO Change to shared pointer
	bool m_playing;
	size_t m_cursor = 0;

	size_t m_audio_cursor = 0;
	int m_minimum_audio;

public:
	explicit AudioPlayer(const AudioData &audioData);

	SDL_AudioStream *connect(SDL_AudioSpec out_spec) {
		if (m_output_stream) throw std::runtime_error("AudioPlayer::connect: already connected");

		if (!SDL_CreateAudioStream(&m_spec, &out_spec))
			throw std::runtime_error("Failed to create audio stream");

		return m_output_stream;
	}

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
