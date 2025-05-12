#pragma once

#include <SDL3/SDL_audio.h>

#include <iostream>


struct AudioData 
{
	const SDL_AudioSpec spec;

	const void *data;
	const uint32_t data_len;

	AudioData(const SDL_AudioSpec &spec, const void *data, const uint32_t data_len)
		: spec(spec), data(data), data_len(data_len) {}

	void print();

	static AudioData loadFromWavFile(const char * path) {
		SDL_AudioSpec r_spec;
		uint8_t *r_data;
		uint32_t r_data_len;
		if(!SDL_LoadWAV(path, &r_spec, &r_data, &r_data_len))
			throw std::runtime_error(std::string("Failed to load wav file.\n[SDL]:") + SDL_GetError());

		return AudioData(r_spec, r_data, r_data_len);
	};
};
