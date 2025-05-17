#pragma once

#include <memory>
#include <stdexcept>

#include "SDL3/SDL.h"
#include <string>

#include "AudioData.h"
#include "AudioPlayer.h"

class AudioVisualizer
{
    const SDL_AudioSpec m_spec = {
        .format = SDL_AUDIO_F32,
        .channels = 1,
        .freq = 44200
    };

    SDL_Window* m_window;
    SDL_Renderer* m_renderer;

    AudioPlayer *m_audio_player;

    SDL_AudioStream* m_input_stream;

public:
    AudioVisualizer(std::string name, int width, int height);

    void connect(AudioPlayer *audio_player) {
        m_audio_player = audio_player;
        m_input_stream = m_audio_player->connect(m_spec);
    }

    SDL_AppResult update() {
        const double now = ((double)SDL_GetTicks()) / 1000.0;  /* convert from milliseconds to seconds. */
        /* choose the color for the frame we will draw. The sine wave trick makes it fade between colors smoothly. */
        const float red = (float) (0.5 + 0.5 * SDL_sin(now));
        const float green = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 2 / 3));
        const float blue = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 4 / 3));
        SDL_SetRenderDrawColorFloat(m_renderer, red, green, blue, SDL_ALPHA_OPAQUE_FLOAT);  /* new color, full alpha. */

        /* clear the window to the draw color. */
        SDL_RenderClear(m_renderer);

        /* put the newly-cleared rendering on the screen. */
        SDL_RenderPresent(m_renderer);

        return SDL_APP_CONTINUE;
    }
};