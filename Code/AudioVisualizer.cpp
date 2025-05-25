#include "Include/AudioVisualizer.h"
#define DJ_FFT_IMPLEMENTATION
#include "dj_fft.h"
#include <numeric>
#include <format>

AudioVisualizer::AudioVisualizer(std::string name, int width_and_height)
    : m_window_width(width_and_height), m_window_height(width_and_height), m_renderType(DRAW_CIRCLE)
{
    SDL_CreateWindowAndRenderer(
        name.c_str(),
        m_window_width, m_window_height,
        SDL_WINDOW_HIGH_PIXEL_DENSITY /* | SDL_WINDOW_ALWAYS_ON_TOP */,
        &m_window, &m_renderer
    );

    if (!m_window || !m_renderer)
        throw std::runtime_error("Failed to create window and renderer");
}

std::vector<float> AudioVisualizer::doFourierTransform(float *samples_start, size_t num_samples)
{
    dj::fft_arg<float> input;
    // Convert to complex values
    input.resize(num_samples);
    for (int i = 0; i < num_samples; i++)
    {
        input[i] = std::complex<float>(samples_start[i], 0.0f);
    }
    // DJ can only handle inputs that have sizes that are a power of two,
    // so we round up to the next power of to and fill with 0s.
    if (std::fmod(log2(num_samples), 1.0) != 0) {
        input.resize(
            std::pow(2, std::ceil(log2(num_samples)))
        );
        for (int i = num_samples; i < input.size(); i++)
            input[i] = 0.0f;
    }
    // Hand over to the library and convert from complex to real values
    auto output = dj::fft1d(input, dj::fft_dir::DIR_FWD);
    std::vector<float> result;
    result.resize(num_samples);
    for (int i = 0; i < num_samples; i++) {
        result[i] = std::abs(output[i]) / static_cast<float>(num_samples / 2);
    }
    // Print the values to the console
    // for (auto &r : result) {
    //     std::cout << r << ", ";
    // }
    // std::cout << std::endl;

    return result;
}

SDL_AppResult AudioVisualizer::update()
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

    // If inside first fft_samples_per_frame samples, use those
    if (m_samples_cursor - m_samples_start < fft_samples_per_frame) {
        m_sample_window_start = m_samples_start;
        m_sample_window_end = m_samples_start + fft_samples_per_frame;
    }
    // If inside last fft_samples_per_frame samples, use those
    else if (m_samples_end - m_samples_cursor < fft_samples_per_frame) {
        m_sample_window_start = m_samples_end - fft_samples_per_frame;
        m_sample_window_end = m_samples_end;
    }
    // Otherwise use the fft_samples_per_frame frames around it
    else {
        m_sample_window_start = m_samples_cursor - (fft_samples_per_frame / 2);
        m_sample_window_end = m_samples_cursor + (fft_samples_per_frame / 2);
    }
    // Calculate the fft
    m_fft_result = doFourierTransform(m_sample_window_start, fft_samples_per_frame);
    // Clear the screen
    SDL_SetRenderDrawColorFloat(m_renderer, 1.0, 1.0, 1.0, SDL_ALPHA_OPAQUE_FLOAT);
    SDL_RenderClear(m_renderer);
    // Present the data in the correct way
    switch (m_renderType) {
        case DRAW_CIRCLE: renderCircle(); break;
        case DRAW_GRAPH: renderGraph(); break;
        default: throw std::runtime_error("Invalid render type");
    }
    // Swap buffers
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
    const float angle_step = 2 * dj::Pi / static_cast<float>(m_fft_result.size() - 1);
    for (int i = 1; i < vertices.size(); i++)
    {
        constexpr float gain = 200;
        vertices[i] = {
            .position = {
                .x = m_window_width/5 * cos( angle_step * (i-1) ) * (1 + gain * m_fft_result[i-1]),
                .y = m_window_height/5 * sin( angle_step * (i-1) ) * (1 + gain * m_fft_result[i-1]),
            },
            .color = {0, 0, 0, 0}, .tex_coord = {0, 0}
        };
    }

    // Fills up the index buffer in a triangle fan configuration (0, 1, 2, 0, 2, 3, 0, 3, 4, ...)
    for (int i = 0; i < m_fft_result.size(); i++)
    {
        indices[i * 3] = 0;
        indices[i * 3 + 1] = i + 1;
        indices[i * 3 + 2] = i + 2;
    }
    indices[m_fft_result.size() * 3 - 1] = 1;

    const SDL_FColor colors[] = {
        {148, 0, 211, 255}, // Violet
        {0, 0, 255, 255},   // Blue
        {0, 255, 0, 255},   // Green
        {255, 255, 0, 255}, // Yellow
        {255, 127, 0, 255}, // Orange
        {255, 0, 0, 255},   // Red
        {0, 0, 0, 255}    // Dark Red
    };

    std::vector<SDL_Vertex> draw_vertices(vertices.size());
    float scale = 1.0f; // start at full size

    for (const auto& color : colors)
    {
        SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);

        for (int i = 0; i < vertices.size(); i++) {
            SDL_Vertex& vertex = draw_vertices[i];

            vertex.color = color;

            // Apply decreasing scale to shrink the geometry
            float scaled_x = vertices[i].position.x * scale;
            float scaled_y = vertices[i].position.y * scale;

            // Center on window
            vertex.position = {
                scaled_x + m_window_width / 2.0f,
                -scaled_y + m_window_height / 2.0f
            };

            vertex.tex_coord = {0.0f, 0.0f}; // Set if not using textures
        }

        if (SDL_RenderGeometry(m_renderer, nullptr, draw_vertices.data(), draw_vertices.size(), indices.data(), indices.size()) < 0) {
            std::cerr << SDL_GetError() << std::endl;
        }

        scale *= 0.9f; // shrink further for next layer
    }
}

void AudioVisualizer::renderGraph()
{
    std::vector<SDL_Vertex> vertices;
    // Get rid of mirror frequencies
    vertices.resize(m_fft_result.size() / 2);
    // Convert to Screen coordinates
    for (int i = 0; i < vertices.size(); i++) {
        vertices[i] = {
            .position = {
                .x = static_cast<float>(m_window_width) / vertices.size() * i, // if one is float, all are float
                .y = m_window_height - static_cast<float>(m_window_height) / 4 * (1 + 200 * m_fft_result[i])
            }
        };
    }

    SDL_SetRenderDrawColor(m_renderer, 255, 0.0f, 0.0f, 255);

    for (int i=0; i<vertices.size()-1; i++) {
        SDL_RenderLine(m_renderer, vertices[i].position.x, vertices[i].position.y, vertices[i+1].position.x, vertices[i+1].position.y);
    }
}