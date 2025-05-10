#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <format>
#include <string>

struct AudioData 
{
	std::string name;
	uint32_t sampleRate;			// Frequency [Hz]
	uint16_t bitsPerSample;			// Size [Bits]
	uint16_t nChannels;				// Number of channels

	std::vector<uint8_t> samples;

	AudioData(
		std::string name,
		uint32_t sampleRate,
		uint16_t bitsPerSample,
		uint16_t nChannels,
		size_t   dataSize
	)
		: name(name), sampleRate(sampleRate), bitsPerSample(bitsPerSample), nChannels(nChannels), samples(dataSize)
	{
		samples.resize(dataSize);
	};

	void print() {
		std::cout << std::format("Sample Rate:\t{} Hz",		sampleRate)		<< std::endl;
		std::cout << std::format("Bits / Sample:\t{} Bits",	bitsPerSample)	<< std::endl;
		std::cout << std::format("Channels:\t{}",			nChannels)		<< std::endl;
		std::cout << std::format("Size:    \t{} Bytes",		samples.size())		<< std::endl;
		std::cout << std::format(
			"Duration:\t{} sec",
			samples.size() / (sampleRate * (bitsPerSample * nChannels) / 8))
			<< std::endl;
	}
};