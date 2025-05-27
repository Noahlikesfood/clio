#pragma once

#include <array>
#include <memory>
#include <stdexcept>

#include "SDL3/SDL.h"
#include <string>

#include "AudioData.h"
#include "AudioPlayer.h"
#include "dj_fft.h"

typedef enum {
    DRAW_CIRCLE = 0,
    DRAW_GRAPH = 1,
    DRAW_MAX
} DRAW_MODE;

#define FFT_SAMPLES_PER_FRAME 1024  // How many samples should get analyzed
#define NUM_CIRCLES 4               // How many Faded out circles do you want?

class AudioVisualizer
{
    SDL_Window* m_window;
    const int m_window_width;
    const int m_window_height;

    SDL_Renderer* m_renderer;

    std::shared_ptr<AudioData> m_audio_data;
    float *m_samples_cursor = nullptr;
    float *m_samples_start = nullptr;
    float *m_samples_end = nullptr;

    float *m_sample_window_start = nullptr;
    float *m_sample_window_end = nullptr;

    std::array<float, FFT_SAMPLES_PER_FRAME> m_fft_result;
    std::array<float, FFT_SAMPLES_PER_FRAME> smoothed_samples;

    void renderCircle();
    void renderGraph();
    void renderBackground();
    uint8_t m_renderType;
    void doFourierTransform(float *samples_start, size_t num_samples);

    void smooth_fft_result(uint8_t window_size);

    std::array<std::array<float, NUM_CIRCLES>, NUM_CIRCLES> m_sin_values;

public:
    AudioVisualizer(std::string name, int width_and_height);

    void print() {
        std::cout << "[Visualizer]\n";
        std::cout << std::format("\tSize:             \t{}, {}\n", m_window_width, m_window_height);
        std::cout << std::format("\tSamples per frame:\t{}\n", m_fft_result.size());
        std::cout << std::format("\tRender Type:      \t{}\n", m_renderType);
        std::cout << "\tSin Offsets:     \t";
        for (auto& arr : m_sin_values) {
            std::cout << "(";
            for (auto& f : arr) {
                std::cout << f << ", ";
            }
            std::cout << "), ";
        }
        std::cout << std::endl;
        SDL_BlendMode mode;
        SDL_GetRenderDrawBlendMode(m_renderer, &mode);
        std::cout << "\tBlend mode:      \t" << static_cast<int>(mode) << std::endl;
        std::cout << std::endl;
    }

    void setAudioData(const std::shared_ptr<AudioData> &audioData) {
        if (audioData->getSampleCount() < m_fft_result.size())
            throw std::runtime_error("AudioData is too small to analize");
        m_audio_data = audioData;
        m_samples_start = audioData->getStart();
        m_samples_end = audioData->getEnd();
    }
    void resetAudioData() {
        m_audio_data.reset();
        m_samples_start = nullptr;
        m_samples_end = nullptr;
        m_fft_result.fill(0.0f);
    }
    void cycleRenderStyle() {
        m_renderType++;
        if (m_renderType == DRAW_MAX)
            m_renderType = DRAW_CIRCLE;
    }

    SDL_AppResult update();
};
