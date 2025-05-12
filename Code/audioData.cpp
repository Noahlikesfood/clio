#include "audioData.h"

#include <format>

void AudioData::print() {
    std::cout << std::format("Sample Rate:\t{} Hz", spec.freq) << std::endl;
    std::cout << std::format("Channels:\t{}", spec.channels)	<< std::endl;
    std::cout << std::format("Size:    \t{} Bytes", data_len) << std::endl;
}
