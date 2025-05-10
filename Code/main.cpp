#include "internal.h"

#include "internal.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"


#include <algorithm>


class AudioPlayer
{
private:
	AudioData &audioData;
	size_t cursor = 0;

	ma_device device;

	// std::vector<AudioSubscriber*> subscribers;

	// Called by the miniaudio each time new frames are needed
	static void dataCallback(
		ma_device* pDevice,
		void* pOutput, const void* pInput,
		uint32_t frameCount
	) {
		// pDevice saves instance of this class
		AudioPlayer* self = reinterpret_cast<AudioPlayer*>(pDevice->pUserData);

		// Display and Play the samples
		self->onCallback(pOutput, frameCount);
	}

	void onCallback(void* pOutput, uint32_t frameCount)
	{
		// Get Requested Bytes
		size_t bytesPerFrame = ma_get_bytes_per_frame(
			device.playback.format,
			device.playback.channels
		);
		size_t bytesRequested = frameCount * bytesPerFrame;
		// Bounds checking
		size_t bytesAvailable = audioData.samples.size() - cursor;
		size_t bytesToCopy = std::min(bytesRequested, bytesAvailable);

		// Actually copy the data
		std::memcpy(pOutput, audioData.samples.data() + cursor, bytesToCopy);

		if (bytesToCopy < bytesRequested)
		{
			std::memset((uint8_t*)pOutput + bytesToCopy, 0, bytesRequested - bytesToCopy);

        	ma_device_stop(&device);
		}


		// Update the subscribers
		// for (auto &sub : subscribers)
// 			sub->update(cursor, frameCount);

		// Advance cursor
		cursor += bytesToCopy;
	}

public:
	AudioPlayer(
		AudioData &audioData
	) : audioData(audioData)
	{
		ma_device_config config = ma_device_config_init(ma_device_type_playback);
		switch (audioData.bitsPerSample)
		{
		case 8:
			config.playback.format = ma_format_u8;
			break;
		case 16:
			config.playback.format = ma_format_s16;
			break;
		case 24:
			config.playback.format = ma_format_s24;
			break;
		case 32:
			config.playback.format = ma_format_s32;
			break;
		default:
			throw std::runtime_error("Playback for this format is not supported");
		}
		config.sampleRate = audioData.sampleRate;
		config.playback.channels = audioData.nChannels;
		config.dataCallback = AudioPlayer::dataCallback;
		config.pUserData = this;

		if (ma_device_init(nullptr, &config, &device) != MA_SUCCESS)
			throw std::runtime_error("Failed to init audio device");
	}

	void play() {
		if (ma_device_start(&device) != MA_SUCCESS) {
			printf("Failed to start playback device.\n");
			ma_device_uninit(&device);
			return;
		}
		std::getchar();
	}

	~AudioPlayer() {
		ma_device_uninit(&device);
	}
};

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

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

std::shared_ptr<AudioData> g_audioData;
std::shared_ptr<SDL_AudioPlayer> g_audioPlayer;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_SetAppMetadata("Musik Visualizer", "1.0", "com.noah.engled");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Musik Visualizer", 640, 480, 0, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

	// try {
	// 	WavFile w("../Musik/test.wav");
	// 	g_audioData = std::make_shared<AudioData>(w.getAudioData());
	// 	g_audioPlayer = std::make_shared<SDL_AudioPlayer>(*g_audioData);
	// 	g_audioData->print();
	// } catch (std::exception& e) {
	// 	std::cerr << e.what() << std::endl;
	// }

    return SDL_APP_CONTINUE;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
	// Quitting the program
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
	// File drop event
	if (event->type == SDL_EVENT_DROP_FILE) {
		std::cout << event->drop.data << std::endl;
		try {
			WavFile w(event->drop.data);
			g_audioData = std::make_shared<AudioData>(w.getAudioData());
			g_audioPlayer = std::make_shared<SDL_AudioPlayer>(*g_audioData);
		} catch (const std::exception &e) {
			std::cout << e.what() << std::endl;
		}


		// SDL_free((void*)event->drop.data); // Causes corruption fsr
	}
	// Play and Pause
	else if (event->type == SDL_EVENT_KEY_DOWN) {
		if (!g_audioPlayer) return SDL_APP_CONTINUE;
		if (event->key.scancode == SDL_SCANCODE_SPACE) {
			g_audioPlayer->togglePlayback();
		}
	}

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
	if (g_audioPlayer)
		g_audioPlayer->feedSamples();

	return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */
}

