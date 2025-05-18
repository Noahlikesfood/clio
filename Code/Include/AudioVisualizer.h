#pragma once

#include <memory>
#include <stdexcept>

#include "SDL3/SDL.h"
#include <string>

#include "AudioData.h"
#include "AudioPlayer.h"

class AudioVisualizer
{
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;

    AudioPlayer *m_audio_player;

public:
    AudioVisualizer(std::string name, int width, int height);

    SDL_AppResult update()
    {
        SDL_SetRenderDrawColorFloat(m_renderer, 69, 69, 69, SDL_ALPHA_OPAQUE_FLOAT);  /* new color, full alpha. */

        SDL_RenderClear(m_renderer);

        SDL_RenderPresent(m_renderer);

        return SDL_APP_CONTINUE;
    }
};