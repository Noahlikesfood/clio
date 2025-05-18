#include "AudioData.h"

#include <format>

void AudioData::print() {
    std::cout << std::format("Path:    \t{}", path.string()) << std::endl;
    std::cout << std::format("Sample Rate:\t{} Hz", spec.freq) << std::endl;
    std::cout << std::format("Channels:\t{}", spec.channels)	<< std::endl;
    std::cout << std::format("Size:    \t{} Bytes", data.size()) << std::endl;
    int sample_size = 0;
    switch (spec.format) {
        case SDL_AUDIO_F32:
            sample_size = 4;
            std::cout << std::format("Format:  \tF32") << std::endl; break;
        case SDL_AUDIO_U8:
            sample_size = 1;
            std::cout << std::format("Format:  \tU8") << std::endl; break;
        case SDL_AUDIO_S16:
            sample_size = 2;
            std::cout << std::format("Format:  \tS16") << std::endl; break;
        case SDL_AUDIO_S32:
            sample_size = 4;
            std::cout << std::format("Format:  \tS32") << std::endl; break;
        default:
            throw std::runtime_error("Invalid Audio Format State");
    }
    std::cout << std::format("Duration:\t{:1} Seconds",
        static_cast<float>(data.size()) / static_cast<float>(sample_size * spec.freq * spec.channels)
    ) << std::endl;
    std::cout << std::endl;
}

AudioData AudioData::loadFromWavFile(std::filesystem::path &path)
    {
        // Allocate space for the data that will be read
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
        RIFFChunk chunk = {};
        size_t filesize = 0;

        // Verify RIFF Header
        infile.read(&chunk_name[0], 4);
        if (strcmp(&chunk_name[0], "RIFF") != 0)
            throw std::runtime_error("Invalid wav file.");

        // Get filesize and validate if necessary
        infile.read(reinterpret_cast<char*>(&filesize), 4);
        if (filesize == 0xffffffff || filesize == 0x0000)
            filesize = std::filesystem::file_size(path) - 8;

        // Verify WAVE Signature
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

        // Convert everything to normal F32
        SDL_AudioSpec target_spec = {
            .format = SDL_AUDIO_F32,
            .channels = 1,
            .freq = 44100,
        };
        std::vector<uint8_t> target_data;
        uint8_t *target_buffer;
        int target_buffer_size;
        if (!SDL_ConvertAudioSamples(
            &r_spec,
            r_data.data(),
            r_data.size(),
            &target_spec,
            &target_buffer,
            &target_buffer_size
        )) {
            throw std::runtime_error("Failed to convert audio data.");
        }
        target_data.assign(target_buffer, target_buffer + target_buffer_size);


		return AudioData{std::move(path), target_spec, std::move(target_data)};
	};