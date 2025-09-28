#include "audio_manager.hpp"
#include "utils.hpp"
#include <regex>

namespace sysfetch::audio {

void gather_mpris_info(AudioInfo& audio) noexcept {
    if (auto mpris_info = utils::execute_command("playerctl metadata --format '{{artist}} - {{title}}' 2>/dev/null"); mpris_info && *mpris_info != "No players found") {
        if (auto pos = mpris_info->find(" - "); pos != std::string::npos) {
            audio.artist = mpris_info->substr(0, pos);
            audio.current_track = mpris_info->substr(pos + 3);
        } else {
            audio.current_track = *mpris_info;
        }

        if (auto player = utils::execute_command("playerctl metadata --format '{{playerName}}' 2>/dev/null"); player) {
            audio.player = *player;
        }

        if (auto status = utils::execute_command("playerctl status 2>/dev/null"); status) {
            audio.is_playing.store(*status == "Playing", std::memory_order_relaxed);
        }

        if (auto position_str = utils::execute_command("playerctl position 2>/dev/null"); position_str) {
            try {
                audio.position.store(static_cast<int>(std::stod(*position_str)), std::memory_order_relaxed);
            } catch (...) {
                audio.position.store(0, std::memory_order_relaxed);
            }
        }

        audio.last_updated = std::chrono::steady_clock::now();
    }
}

void gather_spotify_info(AudioInfo& audio) noexcept {
    const std::array<std::string_view, 3> spotify_commands = {
        "dbus-send --print-reply --dest=org.mpris.MediaPlayer2.spotify /org/mpris/MediaPlayer2 org.freedesktop.DBus.Properties.Get string:org.mpris.MediaPlayer2.Player string:Metadata 2>/dev/null",
        "qdbus org.mpris.MediaPlayer2.spotify /org/mpris/MediaPlayer2 org.freedesktop.DBus.Properties.Get org.mpris.MediaPlayer2.Player Metadata 2>/dev/null",
        "gdbus call --session --dest org.mpris.MediaPlayer2.spotify --object-path /org/mpris/MediaPlayer2 --method org.freedesktop.DBus.Properties.Get org.mpris.MediaPlayer2.Player Metadata 2>/dev/null"
    };

    for (const auto& cmd : spotify_commands) {
        if (auto result = utils::execute_command(cmd); result && !result->empty()) {
            static const std::regex title_regex{R"(xesam:title.*?variant.*?string\s*\"([^\"]+)\")"};
            static const std::regex artist_regex{R"(xesam:artist.*?variant.*?array.*?string\s*\"([^\"]+)\")"};

            std::smatch match;
            if (std::regex_search(*result, match, title_regex)) {
                audio.current_track = match[1].str();
            }
            if (std::regex_search(*result, match, artist_regex)) {
                audio.artist = match[1].str();
            }

            if (!audio.current_track.empty()) {
                audio.player = "Spotify";
                audio.last_updated = std::chrono::steady_clock::now();
                break;
            }
        }
    }
}

void gather_vlc_info(AudioInfo& audio) noexcept {
    if (auto vlc_info = utils::execute_command("vlc-ctrl info 2>/dev/null"); vlc_info && !vlc_info->empty()) {
        std::istringstream iss{*vlc_info};
        std::string line;

        while (std::getline(iss, line)) {
            if (line.find("Title:") != std::string::npos) {
                if (auto pos = line.find(':'); pos != std::string::npos) {
                    audio.current_track = utils::trim_string(line.substr(pos + 1));
                }
            } else if (line.find("Artist:") != std::string::npos) {
                if (auto pos = line.find(':'); pos != std::string::npos) {
                    audio.artist = utils::trim_string(line.substr(pos + 1));
                }
            }
        }

        if (!audio.current_track.empty()) {
            audio.player = "VLC";
            audio.last_updated = std::chrono::steady_clock::now();
        }
    }
}

void gather_firefox_audio_info(AudioInfo& audio) noexcept {
    if (auto firefox_tabs = utils::execute_command("pacmd list-sink-inputs | grep -A 30 'application.name = \"Firefox\"' | grep 'media.name' 2>/dev/null"); firefox_tabs && !firefox_tabs->empty()) {
        static const std::regex media_regex{R"(media\.name = \"([^\"]+)\")"};
        std::smatch match;

        if (std::regex_search(*firefox_tabs, match, media_regex)) {
            audio.current_track = match[1].str();
            audio.player = "Firefox";
            audio.last_updated = std::chrono::steady_clock::now();
        }
    }
}

void gather_all_audio_sources(AudioInfo& audio) noexcept {
    gather_mpris_info(audio);

    if (audio.current_track.empty()) {
        gather_spotify_info(audio);
    }

    if (audio.current_track.empty()) {
        gather_vlc_info(audio);
    }

    if (audio.current_track.empty()) {
        gather_firefox_audio_info(audio);
    }

    if (audio.current_track.empty()) {
        if (auto fallback = get_current_song(); fallback) {
            audio.current_track = *fallback;
            audio.last_updated = std::chrono::steady_clock::now();
        }
    }
}

}