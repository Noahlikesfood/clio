#include "Include/AudioVisualizer.h"
#define DJ_FFT_IMPLEMENTATION
#include "dj_fft.h"
#include <numeric>
#include <format>

AudioVisualizer::AudioVisualizer(std::string name, int width, int height)
    : m_window_width(width), m_window_height(height)
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
    // for (auto &r : result) {
    //     std::cout << r << ", ";
    // }
    // std::cout << std::endl;

    return result;
}

SDL_AppResult AudioVisualizer::update() {
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

    SDL_SetRenderDrawColorFloat(m_renderer, 1.0, 1.0, 1.0, SDL_ALPHA_OPAQUE_FLOAT);
    SDL_RenderClear(m_renderer);

    renderCircle();

    SDL_RenderPresent(m_renderer);
    return SDL_APP_CONTINUE;
}

void AudioVisualizer::renderCircle()
{
    std::vector<SDL_Vertex> vertices;
    std::vector<int> indices;

    vertices.resize(m_fft_result.size() + 1);
    indices.resize(m_fft_result.size() * 3);

    // Fills up the vertex buffer with the vertices of a circle
    vertices[0] = {{0, 0}, {0, 0}, {0, 0}};
    const float angle_step = 2 * dj::Pi / static_cast<float>(m_fft_result.size());
    for (int i = 1; i < vertices.size(); i++)
    {
        vertices[i] = {
            .position = {
                .x = 50 * cos( angle_step * (i-1) ),
                .y = 50 * sin( angle_step * (i-1) ),
            },
            .color = {0, 0}, .tex_coord = {0, 0}
        };
    }
    for (auto &vertex : vertices)
        vertex.position = {vertex.position.x + m_window_width / 2, -vertex.position.y + m_window_height / 2};

    // Fills up the index buffer in a triangle fan configuration (0, 1, 2, 0, 2, 3, 0, 3, 4, ...)
    for (int i = 0; i < m_fft_result.size(); i++)
    {
        indices[i * 3] = 0;
        indices[i * 3 + 1] = i + 1;
        indices[i * 3 + 2] = i + 2;
    }
    indices[m_fft_result.size() * 3 - 1] = 1;

    SDL_SetRenderDrawColor(m_renderer, 255.0f, 0.0f, 0.0f, 255);

    if (!SDL_RenderGeometry(m_renderer, nullptr, vertices.data(), vertices.size(), indices.data(), indices.size())) {
        std::cerr << SDL_GetError() << std::endl;
    }

}