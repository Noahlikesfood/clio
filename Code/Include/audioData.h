#pragma once

#include <SDL3/SDL_audio.h>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <unordered_map>

struct AudioData 
{
    std::filesystem::path path;
	SDL_AudioSpec spec;

    // Used as a container for samples of any type
    std::vector<uint8_t> data;

    // rValue constructor (used in loadFromWavFile)
    AudioData(std::filesystem::path &&path, SDL_AudioSpec spec, std::vector<uint8_t> &&data)
        : path(std::move(path)), spec(spec), data(std::move(data)) {};
    // Move constructor
    AudioData(AudioData &&other) noexcept
    : path(std::move(other.path)), spec(other.spec), data(std::move(other.data)) {}
    // Move assignment operator
    AudioData &operator=(AudioData &&other) noexcept {
        AudioData temp(std::move(other));
        std::swap(*this, temp);
        return *this;
    }

    // Delete the copy constructors
	AudioData(const AudioData &) = delete;
    AudioData& operator=(const AudioData &) = delete;

	~AudioData() {}
	void print();

	//    23,527,424
	// 4,294,967,295

	static AudioData loadFromWavFile(std::filesystem::path &path);
};
