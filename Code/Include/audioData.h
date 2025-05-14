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

	static AudioData loadFromWavFile(std::filesystem::path &path)
    {
		SDL_AudioSpec r_spec;
        std::vector<uint8_t> r_data;

        struct RIFFChunk {
            uint32_t chunk_size;
            size_t data_start;
        };

        // Open file
        std::ifstream infile;
        infile.exceptions(std::ios::failbit | std::ios::badbit);
        infile.open(path, std::ios::binary);


        // Allocate variables to read into
        std::unordered_map<std::string, RIFFChunk> chunks;
        std::string chunk_name(4, '\0');
        RIFFChunk chunk;
        size_t filesize = 0;

        // Verify RIFF Header
        infile.read(&chunk_name[0], 4);
        if (strcmp(&chunk_name[0], "RIFF") != 0)
            throw std::runtime_error("Invalid wav file.");

        // Get filesize and validate if neccesary
        infile.read(reinterpret_cast<char*>(&filesize), 4);
        if (filesize == 0xffffffff || filesize == 0x0000)
            filesize = std::filesystem::file_size(path) - 8;

        // Verify WAVE Signeature
        infile.read(&chunk_name[0], 4);
        if (strcmp(&chunk_name[0], "WAVE") != 0)
            throw std::runtime_error("Invalid wav file.");


        // Get Chunks
        while (static_cast<size_t>(infile.tellg()) < filesize)
        {
            // Reset chunk to read into
            chunk = { 0 };
            // Read in name, size and beginning of data
            infile.read(&chunk_name[0], 4);
            infile.read(reinterpret_cast<char*>(&chunk.chunk_size), sizeof(chunk.chunk_size));
            chunk.data_start = infile.tellg();
            // Copy the pair into the map and go to the next chunk
            chunks.insert({ chunk_name, chunk });
            infile.seekg(chunk.chunk_size, std::ios::cur);
        }

        // Extract Data from fmt\0 chunk     
        uint16_t sampleType;
        uint16_t nChannels;
        uint32_t sampleRate;
        uint32_t bytesPerSec;
        uint16_t bytesPerBlock;
        uint16_t bitsPerSample;

        chunk = chunks.at("fmt ");
        infile.seekg(chunk.data_start, std::ios::beg);
        infile.read(reinterpret_cast<char*>(&sampleType), sizeof(sampleType));
        infile.read(reinterpret_cast<char*>(&nChannels), sizeof(nChannels));
        infile.read(reinterpret_cast<char*>(&sampleRate), sizeof(sampleRate));
        infile.read(reinterpret_cast<char*>(&bytesPerSec), sizeof(bytesPerSec));
        infile.read(reinterpret_cast<char*>(&bytesPerBlock), sizeof(bytesPerBlock));
        infile.read(reinterpret_cast<char*>(&bitsPerSample), sizeof(bitsPerSample));

        r_spec.freq     = static_cast<int>(sampleRate);
        r_spec.channels = static_cast<int>(nChannels);
        switch (sampleType)
        {
        case 1:
            switch (bitsPerSample)
            {
            case 8:
                r_spec.format = SDL_AUDIO_U8; break;
            case 16:
                r_spec.format = SDL_AUDIO_S16; break;
            case 32:
                r_spec.format = SDL_AUDIO_S32; break;
            default:
                throw std::runtime_error("Unsupported PCM bit depth of " + std::to_string(bitsPerSample));
            }
            break;
        case 3:
            switch (bitsPerSample)
            {
            case 32:
                r_spec.format = SDL_AUDIO_F32; break;
            default:
                throw std::runtime_error("Unsupported float bit depth of " + std::to_string(bitsPerSample));
            }
            break;
        default:
            throw std::runtime_error(std::string("Wav file not supported.\n[SDL]:") + SDL_GetError());
        }


        // Extract samples from data chunk
        chunk = chunks.at("data");
        size_t data_size = chunk.chunk_size;
        if (chunk.chunk_size == 0xFFFFFFFF || chunk.chunk_size == 0x00000000)
        {
            infile.seekg(0, std::ios::end);
            data_size = static_cast<size_t>(infile.tellg()) - chunk.data_start;
        }
        r_data.resize(data_size);
	    infile.seekg(chunk.data_start, std::ios::beg);
	    infile.read(reinterpret_cast<char*>(r_data.data()), data_size);
        r_data.shrink_to_fit();
		return AudioData{std::move(path), r_spec, std::move(r_data)};
	};
};
