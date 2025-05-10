#pragma once

#include "internal.h"

#include <filesystem>
#include <fstream>
#include <unordered_map>

#include "audioData.h"

class WavFile
{
private:

    std::ifstream infile;

    struct RIFFChunk {
        char riffID[5] = { 0 };
        uint32_t chunkSize;
        size_t data;
    };

    std::filesystem::path path;
    size_t fileSize = 0;

    char fileType[5] = { 0 };
    uint16_t sampleType = 0;            // 1: PCM integer, 3: IEEE 754 float
    uint16_t nChannels = 0;             // 
    uint32_t sampleRate = 0;            // 
    uint32_t bytesPerSec = 0;           // Frequency * BytePerBloc
    uint16_t bytesPerBlock = 0;         // NbrChannels * BitsPerSample / 8
    uint16_t bitsPerSample = 0;         // 

    size_t dataSize = 0;

    std::unordered_map<std::string, RIFFChunk> riffChunks;

public:

    size_t getFileSize() {
        return std::filesystem::file_size(this->path);
    }

    WavFile(const std::filesystem::path path)
    {
        // Open file
        this->path = path;
        infile.exceptions(std::ios::failbit | std::ios::badbit);
        infile.open(this->path, std::ios::binary);

        // Get RIFF Header
        char riff[5] = {0};
        infile.read(riff, 4);
        if (strcmp(riff, "RIFF") != 0)
            throw std::runtime_error("Invalid File");

        infile.read(reinterpret_cast<char*>(&fileSize), 4);
        if (fileSize == 0xffffffff || fileSize == 0x0000)
            fileSize = getFileSize() - 8;

        infile.read(fileType, sizeof(fileType) - 1);

        // Get Chunks
        RIFFChunk c;
        while (static_cast<size_t>(infile.tellg()) < fileSize)
        {
            infile.read(c.riffID, 4);
            infile.read(reinterpret_cast<char*>(&c.chunkSize), sizeof(c.chunkSize));
            c.data = infile.tellg();
            // riffChunks.push_back(c);
            std::string key(c.riffID);
            riffChunks[key] = c;
            infile.seekg(c.chunkSize, std::ios::cur);
        }

        // Extract Data from fmt\0 chunk
        c = riffChunks.at("fmt ");
        infile.seekg(c.data, std::ios::beg);
        infile.read(reinterpret_cast<char*>(&sampleType), sizeof(sampleType));
        infile.read(reinterpret_cast<char*>(&nChannels), sizeof(nChannels));
        infile.read(reinterpret_cast<char*>(&sampleRate), sizeof(sampleRate));
        infile.read(reinterpret_cast<char*>(&bytesPerSec), sizeof(bytesPerSec));
        infile.read(reinterpret_cast<char*>(&bytesPerBlock), sizeof(bytesPerBlock));
        infile.read(reinterpret_cast<char*>(&bitsPerSample), sizeof(bitsPerSample));

        // Extract Data from data chunk
        c = riffChunks.at("data");
        dataSize = c.chunkSize;
        if (dataSize == 0xFFFFFFFF || dataSize == 0x00000000)
        {
            infile.seekg(0, std::ios::end);
            dataSize = static_cast<size_t>(infile.tellg()) - c.data;
            // std::println(std::cerr, "Replaced data size with {}\n", dataSize); 
        }
    }

    AudioData getAudioData() 
    {    
        // Compatability check
        if (sampleType != 1)
            throw std::runtime_error("Unsupported Sample Type.");

        // Allocate Space for AudioData
        AudioData audioData(path.string(), sampleRate, bitsPerSample, nChannels, dataSize);
        
        // Get Data
        RIFFChunk chunk = riffChunks.at("data");
        infile.seekg(chunk.data, std::ios::beg);
        infile.read(reinterpret_cast<char*>(audioData.samples.data()), dataSize);

        return audioData;
    }

    void printMetadata() {
        std::cout << std::format("\nRIFF Chunks for {}:", path.filename().string());
        for (auto& chunk : riffChunks) {
            if (chunk.second.chunkSize == 0xFFFFFFFF || chunk.second.chunkSize == 0x00000000)
            {
                std::cout << std::format("{}:\tUntil EOF", chunk.second.riffID);
                continue;
            }
            std::cout << std::format("{}:\t{}", chunk.second.riffID, chunk.second.chunkSize);
        }

        std::cout << std::format("Formatting of {}:", path.filename().string());
        
        std::cout << std::format("\tFile Type \t{}", fileType);
        std::cout << std::format("\tFile Size \t{}", fileSize);
        std::cout << std::format("\tLength    \t{} sec", dataSize / bytesPerSec );
        
        std::cout << std::format("\tData Size \t{}", dataSize);
        std::cout << std::format("\tSample Type \t{}", sampleType);
        std::cout << std::format("\tChannels \t{}", nChannels);
        std::cout << std::format("\tSample Rate \t{} Hz", sampleRate);
        std::cout << std::format("\tBits per Sample\t{} Bit", bitsPerSample);
    }
};