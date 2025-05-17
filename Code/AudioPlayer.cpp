#include "AudioPlayer.h"

AudioPlayer::AudioPlayer(const AudioData &audio_data)
    : m_audio_data(audio_data)
{
    // Open audio device
    m_device_id = SDL_OpenAudioDevice(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &m_spec);
    if (m_device_id == 0)
        throw std::runtime_error(std::string("Failed to initialize audio device.\n[SDL]: ") + SDL_GetError());

    // Create audio stream
    m_playback_stream = SDL_CreateAudioStream(
        &m_audio_data.spec, &m_spec);
    if (m_playback_stream == nullptr)
        throw std::runtime_error(std::string("Failed to bind audio device.\n[SDL]: ") + SDL_GetError());
    // Bind it to the device
    if (!SDL_BindAudioStream(m_device_id, m_playback_stream))
        throw std::runtime_error(std::string("Failed to bind audio stream.\n[SDL]: ") + SDL_GetError());

    // Calculate minimum samples that have to be present int the os buffer
    m_minimum_audio =
        m_spec.channels * m_spec.freq * 4; // 4 is size of 32-Bit float

    // Start playback
    m_playing = true;
    SDL_ResumeAudioDevice(m_device_id);
};

void AudioPlayer::togglePlayback() {
    if (m_playing == true) {
        std::cout << "Pause" << std::endl;
        SDL_PauseAudioDevice(m_device_id);
        m_playing = false;
        return;
    }
    std::cout << "Play" << std::endl;
    SDL_ResumeAudioDevice(m_device_id);
    m_playing = true;
}

void AudioPlayer::feedSamples() {
    if (SDL_GetAudioStreamQueued(m_playback_stream) < m_minimum_audio) {
        std::cout << "Refilling " << SDL_GetAudioStreamQueued(m_playback_stream) << " Bytes" << std::endl;
        if (!SDL_PutAudioStreamData(
            m_playback_stream, m_audio_data.data.data() + m_audio_cursor, m_minimum_audio
        )) {
            throw std::runtime_error(std::string("Failed to put data into the playback stream.\n[SDL]: ") + SDL_GetError());
        }

        if (!SDL_PutAudioStreamData(
            m_output_stream, m_audio_data.data.data() + m_audio_cursor, m_minimum_audio
        )) {
            throw std::runtime_error(std::string("Failed to put data into the output stream.\n[SDL]: ") + SDL_GetError());
        }

        m_audio_cursor += m_minimum_audio;
    }
}

AudioPlayer::~AudioPlayer() {
    SDL_DestroyAudioStream(m_playback_stream);
    SDL_CloseAudioDevice(m_device_id);
};
