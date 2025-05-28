#include "Include/AudioVisualizer.h"
#define DJ_FFT_IMPLEMENTATION
#include "dj_fft.h"
#include <random>
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
    // Error handling
    if (!m_window || !m_renderer)
        throw std::runtime_error("Failed to create window and renderer");
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    // Generate random sin functions for visual flair
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distrib(0, 1);
    for (auto& arr: m_sin_values) {
        for (auto& val: arr) {
            val = distrib(gen);
        }
    }
    // Zero Initialize
    m_fft_result.fill(0.0f);
    smoothed_samples.fill(0.0f);
    print();
}

void AudioVisualizer::doFourierTransform(float *samples_start, size_t num_samples)
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
    for (int i = 0; i < num_samples; i++) {
        m_fft_result[i] = std::abs(output[i]) / static_cast<float>(num_samples / 2);
    }
    // Print the values to the console
    // for (auto &r : result) {
    //     std::cout << r << ", ";
    // }
    // std::cout << std::endl;

    smooth_fft_result(10);
}

SDL_AppResult AudioVisualizer::update()
{
    // If pointer changed, recalculate
    if (m_audio_data && m_audio_data->getCurrent() != m_samples_cursor) {
        // Do Fourier Analysis on the selected window
        m_samples_cursor = m_audio_data->getCurrent();

        // If inside first fft_samples_per_frame samples, use those
        if (m_samples_cursor - m_samples_start < m_fft_result.size()) {
            m_sample_window_start = m_samples_start;
            m_sample_window_end = m_samples_start + FFT_SAMPLES_PER_FRAME;
        }
        // If inside last fft_samples_per_frame samples, use those
        else if (m_samples_end - m_samples_cursor < m_fft_result.size()) {
            m_sample_window_start = m_samples_end - m_fft_result.size();
            m_sample_window_end = m_samples_end;
        }
        // Otherwise use the fft_samples_per_frame frames around it
        else {
            m_sample_window_start = m_samples_cursor - (m_fft_result.size() / 2);
            m_sample_window_end = m_samples_cursor + (m_fft_result.size() / 2);
        }
        // Calculate the fft
        doFourierTransform(m_sample_window_start, m_fft_result.size());
    }
    else {
        for (auto &a : smoothed_samples)
            a *= .7f;
    }

    // Clear the screen
    SDL_SetRenderDrawColor(m_renderer, 28, 28, 28, 255);
    SDL_RenderClear(m_renderer);
    // Present the data in the correct way
    switch (m_renderType) {
        case DRAW_CIRCLE: renderCircles(); break;
        case DRAW_GRAPH: renderGraph(); break;
        default: throw std::runtime_error("Invalid render type");
    }
    // Swap buffers
    SDL_RenderPresent(m_renderer);
    return SDL_APP_CONTINUE;
}

void AudioVisualizer::renderCircles()
{
    std::array<SDL_Vertex, FFT_SAMPLES_PER_FRAME/2 / NUM_CIRCLES + 1> vertices;     // + 1 for midpoint
    std::array<int, (FFT_SAMPLES_PER_FRAME/2 / NUM_CIRCLES) * 3> indices;           // * 3 because triangles
    constexpr int samples_per_circle = FFT_SAMPLES_PER_FRAME/2 / NUM_CIRCLES;

    // Fills up the index buffer in a triangle fan configuration (0, 1, 2, 0, 2, 3, 0, 3, 4, ...)
    for (int i = 0; i < indices.size()/3; i++)
    {
        indices[i * 3] = 0;
        indices[i * 3 + 1] = i + 1;
        indices[i * 3 + 2] = i + 2;
    }
    indices[indices.size() - 1] = 1; // To complete the last triangle

    for (int j = 0; j < NUM_CIRCLES; j++)   // For each circle
    {
        // Fills up the vertex buffer with the vertices of a circle
        vertices[0] = {{0, 0}, {0, 0, 0, .3f}, {0, 0}};
        const float angle_step = 2.0f * dj::Pi / static_cast<float>(vertices.size() - 1);
        float time = SDL_GetTicks() / 1000.0f;  // Current time in seconds (slower looks better)

        // Calculate color for the circles
        m_color = {
            .r =  sinf(j * time * .1f) / 2.f + .5f,
            .g =  cosf(j * time * .1f) / 2.f + .5f,
            .b = -sinf(j * time * .1f) / 2.f + .5f,
            .a = .3f
        };

        for (int i = 1; i < vertices.size(); i++)   // For each point
        {
            constexpr float radius = 80.f;          // Radius of the circle
            float angle = angle_step * i;           // Current angle of the point

            // Calculate offset for visual flair
            float offset = 0.0f;
            for (int q = 0; q < m_sin_values.size(); q++) {
                float freq = q + 1;                                 // Frequency increases
                float phase = m_sin_values[q][j];                      // Phase offset is randomly generated
                offset += sinf(freq * angle + time * phase + phase * (j+1));     // Individual waves kind of "race" against each other
            }
            offset /= m_sin_values.size() * (5 + 1/(j+1) );     // normalize and scale down

            // X and Y-Coordinates
            float x = cosf(angle) * radius * (1 + offset);
            float y = sinf(angle) * radius * (1 + offset);

            // Add circle size component
            constexpr float scalar = 0.3;
            x *= 1 + scalar * j;
            y *= 1 + scalar * j;

            // Add fft component
            float circle_average = 0.0f;
            for (int q = 0; q < samples_per_circle/4; q++) {
                circle_average = smoothed_samples[q + samples_per_circle * j] * 10e5;
            }
            float fft_component = circle_average / samples_per_circle;
            x *= 1 + fft_component;
            y *= 1 + fft_component;

            // Add to array
            vertices[i] = {
                .position = { .x = x, .y = y },
                .color = m_color,
                .tex_coord = {0, 0}
            };
        }
        vertices[0].color = m_color;

        // Transform to screen coordinates
        for (auto& vertex : vertices) {
            vertex.position = {
                .x =  vertex.position.x + static_cast<float>(m_window_width)/2,
                .y = -vertex.position.y + static_cast<float>(m_window_height)/2,
            };
        }

        // Render current circle
        if (!SDL_RenderGeometry(m_renderer, nullptr, vertices.data(), vertices.size(), indices.data(), indices.size())) {
            std::cerr << SDL_GetError() << std::endl;
        }
    }
}

void AudioVisualizer::renderGraph()
{
    // / 2 get rid of mirror frequencies
    std::array<SDL_Vertex, FFT_SAMPLES_PER_FRAME / 2> vertices;
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

void AudioVisualizer::smooth_fft_result(uint8_t radius)
{
    for (int i=1; i<m_fft_result.size(); i++) {
        float average = 0.0f;
        for (int current = -radius; current<radius; current++) {
            int in_bounds = (current + i + m_fft_result.size()) % m_fft_result.size(); // Modulo for negative is just wrong
            average += m_fft_result[in_bounds];
        }
        smoothed_samples[i] = average / static_cast<float>(m_fft_result.size());
        // std::cout << smoothed_samples[i] << ", ";
    }
    // std::cout << std::endl;
}