#pragma once

#include <SDL3/SDL_audio.h>

#include <fstream>
#include <iostream>
#include <filesystem>
#include <unordered_map>

class AudioData
{
    std::filesystem::path m_path;
	SDL_AudioSpec m_spec;

    // Used as a container for samples of any type
    std::vector<float> m_data;
    size_t playback_cursor = 0;

public:
    // rValue constructor (used in loadFromWavFile)
    AudioData(std::filesystem::path &&path, SDL_AudioSpec spec, std::vector<float> &&data)
        : m_path(std::move(path)), m_spec(spec), m_data(std::move(data)) {};
    // Move constructor
    AudioData(AudioData &&other) noexcept
    : m_path(std::move(other.m_path)), m_spec(other.m_spec), m_data(std::move(other.m_data)) {}

    // Delete the copy constructors
	AudioData(const AudioData &) = delete;
    AudioData& operator=(const AudioData &) = delete;

	~AudioData() {}
	void print();

	SDL_AudioSpec *getSpec() { return &m_spec; }
	float *getStart() { return m_data.data(); }
	float *getEnd() { return m_data.data() + m_data.size(); }
	float *getCurrent() {return m_data.data() + playback_cursor;}
	size_t getSampleCount() { return m_data.size(); }
	void advanceCursor(size_t offset) {
		offset /= sizeof(float);
		if (playback_cursor + offset >= m_data.size()) {
			playback_cursor = 0;
		} else {
			playback_cursor += offset;
		}
	}
	static AudioData loadFromWavFile(std::filesystem::path &path);
};
