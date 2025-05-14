#include "audioData.h"

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
    }
    std::cout << std::format("Duration:\t{:1} Seconds",
        static_cast<float>(data.size()) / static_cast<float>(sample_size * spec.freq * spec.channels)
    ) << std::endl;
    std::cout << std::endl;
}
