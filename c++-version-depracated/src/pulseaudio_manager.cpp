#include "audio_manager.hpp"
#include "utils.hpp"
#include <atomic>

namespace sysfetch::audio {

static std::atomic<bool> pulse_ready{false};
static std::string current_sink_name;
static std::atomic<double> current_volume{0.0};

void PulseAudioManager::context_state_callback(pa_context* context, void* userdata) noexcept {
    const auto state = pa_context_get_state(context);

    switch (state) {
        case PA_CONTEXT_READY:
            pulse_ready.store(true, std::memory_order_relaxed);
            break;
        case PA_CONTEXT_FAILED:
        case PA_CONTEXT_TERMINATED:
            pulse_ready.store(false, std::memory_order_relaxed);
            break;
        default:
            break;
    }
}

void PulseAudioManager::sink_info_callback(pa_context* context, const pa_sink_info* sink_info, int eol, void* userdata) noexcept {
    if (eol > 0) return;

    if (sink_info) {
        current_sink_name = sink_info->name ? sink_info->name : "Unknown";
        const auto volume_avg = pa_cvolume_avg(&sink_info->volume);
        const auto volume_percent = static_cast<double>(volume_avg) / PA_VOLUME_NORM * 100.0;
        current_volume.store(volume_percent, std::memory_order_relaxed);
    }
}

PulseAudioManager::PulseAudioManager() {
    mainloop_ = pa_mainloop_new();
    if (mainloop_) {
        api_ = pa_mainloop_get_api(mainloop_);
        context_ = pa_context_new(api_, "sysfetch-pulse");

        if (context_) {
            pa_context_set_state_callback(context_, context_state_callback, nullptr);
            pa_context_connect(context_, nullptr, PA_CONTEXT_NOFLAGS, nullptr);
        }
    }
}

PulseAudioManager::~PulseAudioManager() {
    if (context_) {
        pa_context_disconnect(context_);
        pa_context_unref(context_);
    }

    if (mainloop_) {
        pa_mainloop_free(mainloop_);
    }
}

bool PulseAudioManager::is_valid() const noexcept {
    return context_ && mainloop_ && api_;
}

void PulseAudioManager::gather_audio_info(AudioInfo& audio) noexcept {
    if (is_valid() && pulse_ready.load()) {
        pa_operation* op = pa_context_get_sink_info_list(context_, sink_info_callback, nullptr);
        if (op) {
            while (pa_operation_get_state(op) == PA_OPERATION_RUNNING) {
                pa_mainloop_iterate(mainloop_, 0, nullptr);
            }
            pa_operation_unref(op);
        }

        audio.device_name = current_sink_name;
        audio.volume.store(current_volume.load(), std::memory_order_relaxed);
    }
}

std::optional<std::string> get_current_song() noexcept {
    if (auto song = utils::execute_command("playerctl metadata --format '{{title}}' 2>/dev/null"); song && *song != "No players found") {
        return song;
    }

    if (utils::file_exists("/tmp/spotify_current")) {
        if (auto song = utils::read_file("/tmp/spotify_current"); song) {
            return utils::trim_string(*song);
        }
    }

    if (auto cmus_status = utils::execute_command("cmus-remote -Q 2>/dev/null | grep 'tag title'"); cmus_status) {
        if (auto pos = cmus_status->find("tag title "); pos != std::string::npos) {
            return cmus_status->substr(pos + 10);
        }
    }

    return std::nullopt;
}

std::optional<std::string> get_audio_device() noexcept {
    if (!current_sink_name.empty()) {
        return current_sink_name;
    }

    if (auto device = utils::execute_command("pactl info 2>/dev/null | grep 'Default Sink:' | cut -d: -f2"); device) {
        return utils::trim_string(*device);
    }

    return std::nullopt;
}

double get_audio_volume() noexcept {
    if (auto volume = current_volume.load(); volume > 0) {
        return volume;
    }

    if (auto volume_str = utils::execute_command("pactl list sinks 2>/dev/null | grep -A 10 'State: RUNNING' | grep 'Volume:' | head -1 | awk '{print $5}' | tr -d '%'"); volume_str) {
        try {
            return std::stod(*volume_str);
        } catch (...) {}
    }

    if (auto volume_str = utils::execute_command("amixer get Master 2>/dev/null | grep -oP '\\d+(?=%)' | tail -1"); volume_str) {
        try {
            return std::stod(*volume_str);
        } catch (...) {}
    }

    return 0.0;
}

}