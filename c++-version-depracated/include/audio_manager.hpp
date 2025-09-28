#pragma once

#include "types.hpp"
#include <pulse/pulseaudio.h>
#include <pulse/simple.h>
#include <pulse/error.h>

namespace sysfetch::audio {

class PulseAudioManager {
public:
    PulseAudioManager();
    ~PulseAudioManager();

    PulseAudioManager(const PulseAudioManager&) = delete;
    PulseAudioManager& operator=(const PulseAudioManager&) = delete;
    PulseAudioManager(PulseAudioManager&&) = delete;
    PulseAudioManager& operator=(PulseAudioManager&&) = delete;

    [[nodiscard]] bool is_valid() const noexcept;
    void gather_audio_info(AudioInfo& audio) noexcept;

private:
    pa_mainloop* mainloop_{nullptr};
    pa_mainloop_api* api_{nullptr};
    pa_context* context_{nullptr};
    std::atomic<bool> ready_{false};

    static void context_state_callback(pa_context* context, void* userdata) noexcept;
    static void sink_info_callback(pa_context* context, const pa_sink_info* sink_info, int eol, void* userdata) noexcept;
};

[[nodiscard]] std::optional<std::string> get_current_song() noexcept;
[[nodiscard]] std::optional<std::string> get_audio_device() noexcept;
[[nodiscard]] double get_audio_volume() noexcept;

}