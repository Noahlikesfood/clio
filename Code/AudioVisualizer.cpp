#include "Include/AudioVisualizer.h"
#define DJ_FFT_IMPLEMENTATION
#include "dj_fft.h"
#include <numeric>
#include <format>

AudioVisualizer::AudioVisualizer(std::string name, int width, int height)
{
    SDL_CreateWindowAndRenderer(
        name.c_str(),
        width, height,
        SDL_WINDOW_HIGH_PIXEL_DENSITY /* | SDL_WINDOW_ALWAYS_ON_TOP */,
        &m_window, &m_renderer
    );

    if (!m_window || !m_renderer)
        throw std::runtime_error("Failed to create window and renderer");
}

std::vector<float> AudioVisualizer::doFourierTransform(float *samples_start, float *samples_end)
{
    dj::fft_arg<float> input;
    // Convert to complex values
    for (int i = 0; i < fft_samples_per_frame; i++)
    {
        input.emplace_back(
            std::complex<float>(samples_start[i], 0.0f)
        );
    }
    // Hand over to the library
    auto output = dj::fft1d(input, dj::fft_dir::DIR_FWD);
    std::vector<float> result;
    for (auto &o : output) {
        result.push_back(std::abs(o) / static_cast<float>(fft_samples_per_frame / 2));
    }
    // Print the values to the console
    for (auto &r : result) {
        std::cout << r << ", ";
    }
    std::cout << std::endl;
    // return ig
    return result;
}