//
// Created by noahk on 16.05.2025.
//

#include "Include/AudioVisualizer.h"

AudioVisualizer::AudioVisualizer(std::string name, int width, int height) {
    SDL_CreateWindowAndRenderer(
        name.c_str(),
        width, height,
        SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_ALWAYS_ON_TOP,
        &m_window, &m_renderer
    );

    if (!m_window || !m_renderer)
        throw std::runtime_error("Failed to create window and renderer");
}