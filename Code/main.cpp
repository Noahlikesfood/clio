#include "AudioVisualizer.h"

#include <memory>
#include <iostream>

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

std::shared_ptr<AudioData> g_audioData;
std::shared_ptr<AudioPlayer> g_audioPlayer;
std::unique_ptr<AudioVisualizer> g_audioVisualizer;

// SDL_Window* window;
// SDL_Renderer* renderer;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_SetAppMetadata("Musik Visualizer", "1.0", "com.noah.engled");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // if (!SDL_CreateWindowAndRenderer("Musik Visualizer", 640, 480, 0, &window, &renderer)) {
    //     SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    //     return SDL_APP_FAILURE;
    // }

	g_audioVisualizer = std::make_unique<AudioVisualizer>("test", 400, 400);

    return SDL_APP_CONTINUE;
}

/* Handles all events for all windows since there will only be one song playing at a time */
/* If I was to add multi-window support, appstate would need to be modified*/
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
	// Quitting the program
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
	// File drop event
	if (event->type == SDL_EVENT_DROP_FILE) {
		std::filesystem::path dropped_file_path(event->drop.data);
		std::cout << "path: " << dropped_file_path << std::endl;

		try {
			g_audioData = std::make_shared<AudioData>(
				AudioData::loadFromWavFile(dropped_file_path)
			);
			g_audioPlayer = std::make_shared<AudioPlayer>(*g_audioData);
			g_audioPlayer->print();
		} catch (const std::exception &e) {
			g_audioData.reset();
			g_audioPlayer.reset();
			std::cout << e.what() << std::endl;
		}
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
	g_audioVisualizer->update();
	SDL_Delay(17); // ~ 60 Hz
	return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
	g_audioData.reset();
	g_audioPlayer.reset();
}

