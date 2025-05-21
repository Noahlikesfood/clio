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

    std::shared_ptr<AudioData> m_audio_data;
    float *m_samples_cursor = nullptr;
    float *m_samples_start = nullptr;
    float *m_samples_end = nullptr;

    float m_fft_result;

public:
    AudioVisualizer(std::string name, int width, int height);

    void setAudioData(const std::shared_ptr<AudioData> &audioData) {
        if (audioData->getSampleCount() < fft_samples_per_frame)
            throw std::runtime_error("AudioData is too small to analize");
        m_audio_data = audioData;
        m_samples_start = audioData->getStart();
        m_samples_end = audioData->getEnd();
    }

    SDL_AppResult update()
    {
        // If there is no data, draw a black screen
        if (!m_audio_data) {
            SDL_SetRenderDrawColorFloat(m_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE_FLOAT);
            SDL_RenderClear(m_renderer);
            SDL_RenderPresent(m_renderer);
            return SDL_APP_CONTINUE;
        }
        // If pointer has not changed, return
        if (m_audio_data->getCurrent() == m_samples_cursor)
            return SDL_APP_CONTINUE;
        // Do Fourier Analysis on the selected window
        m_samples_cursor = m_audio_data->getCurrent();
        if (m_samples_cursor+fft_samples_per_frame > m_samples_end) {
            m_fft_result = doFourierTransform(m_samples_cursor, m_samples_end);
        } else {
            m_fft_result = doFourierTransform(m_samples_cursor, m_samples_cursor+fft_samples_per_frame);
        }

        // std::cout << m_fft_result << std::endl;

        SDL_SetRenderDrawColorFloat(m_renderer, 1.0, 1.0, 1.0, SDL_ALPHA_OPAQUE_FLOAT);
        SDL_RenderClear(m_renderer);
        SDL_RenderPresent(m_renderer);
        return SDL_APP_CONTINUE;
    }

    float doFourierTransform(float *samples_start, float *samples_end) {
        float accumulator = 0.0;
        while (samples_start < samples_end) {
            accumulator += *samples_start;
            samples_start++;
        }
        std::cout << accumulator / static_cast<float>(fft_samples_per_frame) << std::endl;
        return accumulator /= static_cast<float>(fft_samples_per_frame);
    }
};