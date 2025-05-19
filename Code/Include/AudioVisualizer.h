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

    size_t fft_samples_per_frame = 1024;

    std::shared_ptr<AudioData> m_audioData;
    float *m_last_position = nullptr;

public:
    AudioVisualizer(std::string name, int width, int height);

    void setAudioData(const std::shared_ptr<AudioData> &audioData) {
        m_audioData = audioData;
    }

    SDL_AppResult update()
    {
        // If there is no data, draw a black screen
        if (!m_audioData) {
            SDL_SetRenderDrawColorFloat(m_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE_FLOAT);
            SDL_RenderClear(m_renderer);
            SDL_RenderPresent(m_renderer);
            return SDL_APP_CONTINUE;
        }
        // If pointer has not changed, return
        // if (m_audioData->getCurrentPosition() == m_last_position)
        //     return SDL_APP_CONTINUE;
        // m_last_position = m_audioData->getCurrentPosition();

        for (int i=0; i<fft_samples_per_frame; i++) {
            std::cout << m_last_position[i] << " ";
        }
        std::cout << std::endl;

        SDL_SetRenderDrawColorFloat(m_renderer, 1.0, 1.0, 1.0, SDL_ALPHA_OPAQUE_FLOAT);
        SDL_RenderClear(m_renderer);
        SDL_RenderPresent(m_renderer);
        return SDL_APP_CONTINUE;
    }
};