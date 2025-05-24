#pragma once

#include <memory>
#include <stdexcept>

#include "SDL3/SDL.h"
#include <string>

#include "AudioData.h"
#include "AudioPlayer.h"
#include "dj_fft.h"

class AudioVisualizer
{
    SDL_Window* m_window;
    const int m_window_width;
    const int m_window_height;

    SDL_Renderer* m_renderer;

    size_t fft_samples_per_frame = 1024;

    std::shared_ptr<AudioData> m_audio_data;
    float *m_samples_cursor = nullptr;
    float *m_samples_start = nullptr;
    float *m_samples_end = nullptr;

    std::vector<float> m_fft_result;

public:
    AudioVisualizer(std::string name, int width, int height);

    void setAudioData(const std::shared_ptr<AudioData> &audioData) {
        if (audioData->getSampleCount() < fft_samples_per_frame)
            throw std::runtime_error("AudioData is too small to analize");
        m_audio_data = audioData;
        m_samples_start = audioData->getStart();
        m_samples_end = audioData->getEnd();
    }
    void resetAudioData() {
        m_audio_data.reset();
        m_samples_start = nullptr;
        m_samples_end = nullptr;
    }

    SDL_AppResult update();

    void renderCircle();

    std::vector<float> doFourierTransform(float *samples_start, float *samples_end);
};
