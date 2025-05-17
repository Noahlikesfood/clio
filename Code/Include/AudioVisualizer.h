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

    SDL_AudioStream* m_stream;

    std::vector<float> m_data;

public:
    AudioVisualizer(std::string name, int width, int height);

    void connect(AudioPlayer *audio_player)
    {
        m_audio_player = audio_player;
        m_data.resize(m_audio_player->m_minimum_audio);
        m_stream = m_audio_player->connect(m_spec);
    }

    SDL_AppResult update()
    {
        if (SDL_GetAudioStreamData(m_stream, m_data.data(), m_data.size() ) == -1)
            throw std::runtime_error("Failed to get audio from stream data");

        double avg = 0;
        for (const float &f : m_data) {
            avg += f;
        }
        avg /= m_data.size();


        SDL_SetRenderDrawColorFloat(m_renderer, avg, avg, avg, SDL_ALPHA_OPAQUE_FLOAT);  /* new color, full alpha. */

        /* clear the window to the draw color. */
        SDL_RenderClear(m_renderer);

        /* put the newly-cleared rendering on the screen. */
        SDL_RenderPresent(m_renderer);

        return SDL_APP_CONTINUE;
    }
};