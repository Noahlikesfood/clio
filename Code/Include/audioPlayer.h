#pragma once

#include "internal.h"
#include "SDL3/SDL_audio.h"
#include "SDL3/SDL_error.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"

static SDL_Window *window = nullptr;
static SDL_Renderer *renderer = nullptr;
static SDL_AudioStream *audio_stream = nullptr;

class SDL_AudioPlayer
{
private:
	SDL_AudioStream *m_stream;
	AudioData &m_audioData;
	bool m_playing;

	// int m_minimum_audio;
	size_t m_cursor;
public:
	SDL_AudioPlayer(AudioData &ad)
	: m_audioData(ad), m_playing(false), m_cursor(0) {
		SDL_AudioSpec spec;
		spec.channels = m_audioData.nChannels;
		spec.freq = static_cast<int>(m_audioData.sampleRate);
		switch (m_audioData.bitsPerSample)
		{
			case 8:
				spec.format = SDL_AUDIO_U8;
				std::cout << "Selected U8 PCM\n";
				break;
			case 16:
				spec.format = SDL_AUDIO_S16;
				std::cout << "Selected S16 PCM\n";
				break;
			case 24:
				throw std::runtime_error("24 Bit PCM Format not supported.\n");
			case 32:
				spec.format = SDL_AUDIO_S32;
				std::cout << "Selected S32 PCM\n";
				break;
			default:
				throw std::runtime_error("Playback for this format is not supported.\n");
		}

		m_stream = SDL_OpenAudioDeviceStream(
			SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
			&spec,
			nullptr, nullptr
		);

		if (!m_stream)
			throw std::runtime_error(std::string("Failed to open audio device.\n[SDL]: ") + SDL_GetError());

		// Minimum is one second of audio
		// m_minimum_audio = static_cast<int>
		// 	(m_audioData.sampleRate * m_audioData.bitsPerSample * m_audioData.nChannels);
	};

	void togglePlayback() {
		if (m_playing == true) {
			std::cout << "Pause" << std::endl;
			SDL_PauseAudioStreamDevice(m_stream);
			m_playing = false;
			return;
		}
		std::cout << "Play" << std::endl;
		SDL_ResumeAudioStreamDevice(m_stream);
		m_playing = true;
	}

	void feedSamples() {
		if (SDL_GetAudioStreamQueued(m_stream) < m_audioData.samples.size()) {
			if (!SDL_PutAudioStreamData(
				m_stream, m_audioData.samples.data(), static_cast<int>(m_audioData.samples.size())
			)) {
				throw std::runtime_error(std::string("Failed to put audio stream data.\n[SDL]:") + SDL_GetError());
			}
		}
	}

	~SDL_AudioPlayer() {
		SDL_DestroyAudioStream(m_stream);
	};
};