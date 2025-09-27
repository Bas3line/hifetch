#pragma once

#include "common.hpp"
#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <algorithm>

namespace sysfetch::display {

class AsciiArtManager {
private:
    static inline std::unordered_map<std::string, std::vector<std::string>> art_collection_ = {
        {"arch", {
            "           ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄           ",
            "         ▄█                   █▄         ",
            "        ▄█  ███████████████████ █▄       ",
            "       ▄█ ███ ▄▄▄▄▄▄▄▄▄▄▄▄▄ ███ █▄      ",
            "      ▄█ ███ █             █ ███ █▄     ",
            "     ▄█ ███ █   ▄███████▄   █ ███ █▄    ",
            "    ▄█ ███ █   ███     ███   █ ███ █▄   ",
            "   ▄█ ███ █   ███  ▄█▄  ███   █ ███ █▄  ",
            "  ▄█ ███ █   ███  █   █  ███   █ ███ █▄ ",
            "  █ ███ █   ███  █ ● █  ███   █ ███ █  ",
            "  █▄███ █   ███  █   █  ███   █ ███▄█  ",
            "   █▄██ █   ███  ▀█▀  ███   █ ██▄█    ",
            "    █▄█ █   ███     ███   █ █▄█      ",
            "     █▄ █   ▀███████▀   █ ▄█        ",
            "      █▄ █             █ ▄█         ",
            "       █▄ ▀▄▄▄▄▄▄▄▄▄▄▄▄▄▀ ▄█          ",
            "        █▄                 ▄█           ",
            "         ▀█▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄█▀           "
        }},
        {"ubuntu", {
            "         _,met$$$$$gg.          ",
            "      ,g$$$$$$$$$$$$$$$P.       ",
            "    ,g$$P\"     \"\"\"Y$$.\"$.      ",
            "   ,$$P'              `$$$.     ",
            "  ',$$P       ,ggs.     `$$b:   ",
            "  `d$$'     ,$P\"'   .    $$$    ",
            "   $$P      d$'     ,    $$P    ",
            "   $$:      $$.   -    ,d$$'    ",
            "   $$;      Y$b._   _,d$P'      ",
            "   Y$$.    `.`\"Y$$$$P\"'         ",
            "   `$$b      \"-.__              ",
            "    `Y$$                        ",
            "     `Y$$.                      ",
            "       `$$b.                    ",
            "         `Y$$b.                 ",
            "            `\"Y$b._             ",
            "                `\"\"\"\"\"         "
        }},
        {"debian", {
            "       _,met$$$$$gg.          ",
            "    ,g$$$$$$$$$$$$$$$P.       ",
            "  ,g$$P\"     \"\"\"Y$$.\"$.      ",
            " ,$$P'              `$$$.     ",
            "',$$P       ,ggs.     `$$b:   ",
            "`d$$'     ,$P\"'   .    $$$    ",
            " $$P      d$'     ,    $$P    ",
            " $$:      $$.   -    ,d$$'    ",
            " $$;      Y$b._   _,d$P'      ",
            " Y$$.    `.`\"Y$$$$P\"'         ",
            " `$$b      \"-.__              ",
            "  `Y$$                        ",
            "   `Y$$.                      ",
            "     `$$b.                    ",
            "       `Y$$b.                 ",
            "          `\"Y$b._             ",
            "              `\"\"\"\"          "
        }},
        {"fedora", {
            "        ,'''''.        ",
            "       |   ,.  |       ",
            "       |  |  '_'       ",
            "  ,....|  |..          ",
            " .    .    '||,        ",
            "|  '||.          ''.   ",
            " '..    ||.        '|  ",
            "   ''......''     | |  ",
            "           ||.....' |  ",
            "             ''||   |  ",
            "                |  |   ",
            "               .'||'   ",
            "           ...|''      ",
            "      ''''''''         "
        }},
        {"gentoo", {
            " _-----_     ",
            "(       \\    ",
            "\\    0   \\   ",
            " \\        )  ",
            " /      _/   ",
            "(     _-     ",
            "\\____-      "
        }},
        {"manjaro", {
            "██████████████████  ████████     ",
            "██████████████████  ████████     ",
            "██████████████████  ████████     ",
            "██████████████████  ████████     ",
            "████████            ████████     ",
            "████████  ████████  ████████     ",
            "████████  ████████  ████████     ",
            "████████  ████████  ████████     ",
            "████████  ████████  ████████     ",
            "████████  ████████  ████████     ",
            "████████  ████████  ████████     ",
            "████████  ████████  ████████     ",
            "████████  ████████  ████████     "
        }},
        {"mint", {
            " MMMMMMMMMMMMMMMMMMMMMMMMMmds+.     ",
            " MMm----::-://////////////oymNMd+`  ",
            " MMd      /++                -sNMd: ",
            " MMNso/`  dMM    `.::-. .-::.` .hMN:",
            " ddddMMh  dMM   :hNMNMNhNMNMNh: `NMm",
            "     NMm  dMM  .NMN/-+MMM+-/NMN` dMM",
            "     NMm  dMM  -MMm  `MMM   dMM. dMM",
            "     NMm  dMM  -MMm  `MMM   dMM. dMM",
            "     NMm  dMM  .mmd  `mmm   yMM. dMM",
            "     NMm  dMM`  ..`   ...   ydm. dMM",
            "     hMM- +MMd/-------...-:sdds  dMM",
            "     -NMm- :hNMNNNmdddddddddy/`  dMM",
            "      -dMNs-``-::::--------.``    dMM",
            "       `/dMNmy+/:-------------:/yMMM",
            "          ./ydNMMMMMMMMMMMMMMMMMMMMM"
        }},
        {"void", {
            "                __.;=====;.__               ",
            "            _.=+==++=++=+=+===;.            ",
            "             -=+++=+===+=+=+++++=_          ",
            "        .     -=:``     `--==+=++==.       ",
            "       _vi,    `            --+=++++:      ",
            "      .uvnvi.       _._       -==+==+.     ",
            "     .vvnvnI`    .;==|==;.     :|=||=|.    ",
            "     +uvnvnvi  _._|=||=+|=_=_   :||=|=,    ",
            "     +vnvnvns.;=:::=|=|===:.`  .||=||=,    ",
            "     -vnvnvn==+::=+=|=|=+=      :||=||=,   ",
            "      +vnvnvn]+]=  =+|=]]=-     :||=||=.   ",
            "      ]pvnvnvi+:.;=`=+=]=-       :|=||=|.  ",
            "       +vnvnvns.;++`=]=-         :|=||=+.  ",
            "       .}vnvnvnsg;   `=]=-        :|=||=[  ",
            "        +nvnvnvi,     `=]=-       ;]]=]={  ",
            "         .~nvnvni,      `=]=-      :+=]=-  ",
            "           .]vnvni,       `=+=     .___    ",
            "             ,]vnI,         .                "
        }},
        {"dragon", {
            "      \\||/    ",
            "      |  @___oo",
            "      /  (___,|)",
            "     /    \\  /|",
            "    (      | |",
            "    |      | |",
            "    |______|_|",
            "    (________)/"
        }},
        {"tux", {
            "        .---.        ",
            "       /     \\       ",
            "       \\.@-@./       ",
            "       /`\\_/`\\       ",
            "      //  _  \\\\      ",
            "     | \\     )|_     ",
            "    /`\\_`>  <_/ \\    ",
            "    \\__/'---'\\__/    "
        }},
        {"matrix", {
            "█▄─▀█▀─▄█▀▀▀▀▀██▄─▄▄─█▄─▄▄▀█▄─▄█▄─▀─▄█",
            "██─█▄█─████████▀█─▄█▀██─▄─▄██─███─█▄▄▄█",
            "▀▄▄▄▀▄▄▄▀▀▀▀▀▀▀▀▄▄▄▄▄▀▄▄▀▄▄▀▄▄▄▀▄▄▄▀▀▀▀",
            "█ ░░█ █▀▀ █▀▄ █▀▀ █▀█ █▀▄▀█ █▀▀     █▀█",
            "█▄▄█ █▄▄ █▄▀ █▄▄ █▄█ █░▀░█ █▄▄     █▄█",
            " ░░░ ░░░░ ░░░ ░░░ ░░░ ░░░░░░ ░░░     ░░░ ",
            " ░▒▓██  ░▒▓██  ░▒▓██  ░▒▓██  ░▒▓██ ",
            "██▓▒░  ██▓▒░  ██▓▒░  ██▓▒░  ██▓▒░ ",
            " ░▒▓██  ░▒▓██  ░▒▓██  ░▒▓██  ░▒▓██ ",
            "██▓▒░  ██▓▒░  ██▓▒░  ██▓▒░  ██▓▒░ ",
            " ░▒▓██  ░▒▓██  ░▒▓██  ░▒▓██  ░▒▓██ "
        }},
        {"cyber", {
            "╔══════════════════════════════════════╗",
            "║  ▄████▄▄   ▄██  ▄█████▄  ██▄▄████▄   ║",
            "║  ██           ██  ██   ██  ██    ███  ║",
            "║  ██           ██  ██   ██  ██    ███  ║",
            "║  ▀████▄▄     ██  ██▀▀▀██  ██▄▄████▀  ║",
            "║        ██    ██  ██   ██  ██         ║",
            "║        ██    ██  ██   ██  ██         ║",
            "║  ▄████▀▀     ▀█  ██   ██  ██         ║",
            "╚══════════════════════════════════════╝",
            " ░░░░░░░░░░░ SYSTEM OVERRIDE ░░░░░░░░░░░",
            " ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓",
            " ██████████ ACCESS GRANTED ███████████"
        }},
        {"minimal", {
            "   ┌─────────┐   ",
            "   │  SYSTEM │   ",
            "   │   INFO   │   ",
            "   └─────────┘   "
        }},
        {"retro", {
            "╭─────────────────────────────────────╮",
            "│  ░░░░░░░ RETRO COMPUTING ░░░░░░░░░  │",
            "│                                     │",
            "│  ████████  ███████  ████████       │",
            "│      ██    ██       ██    ██       │",
            "│      ██    ███████  ████████       │",
            "│      ██    ██       ██  ██         │",
            "│  ████████  ███████  ██   ████      │",
            "│                                     │",
            "│  ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓  │",
            "╰─────────────────────────────────────╯"
        }}
    };

public:
    static const std::vector<std::string>& get_art(const std::string& name) {
        static const std::vector<std::string> default_art = art_collection_["arch"];

        auto it = art_collection_.find(name);
        return it != art_collection_.end() ? it->second : default_art;
    }

    static const std::vector<std::string>& get_os_art(const std::string& os_name) {
        std::string os_lower = os_name;
        std::transform(os_lower.begin(), os_lower.end(), os_lower.begin(), ::tolower);

        if (os_lower.find("arch") != std::string::npos) return get_art("arch");
        if (os_lower.find("ubuntu") != std::string::npos) return get_art("ubuntu");
        if (os_lower.find("debian") != std::string::npos) return get_art("debian");
        if (os_lower.find("fedora") != std::string::npos) return get_art("fedora");
        if (os_lower.find("gentoo") != std::string::npos) return get_art("gentoo");
        if (os_lower.find("manjaro") != std::string::npos) return get_art("manjaro");
        if (os_lower.find("mint") != std::string::npos) return get_art("mint");
        if (os_lower.find("void") != std::string::npos) return get_art("void");

        return get_art("arch");
    }

    static std::vector<std::string> get_available_arts() {
        std::vector<std::string> names;
        for (const auto& pair : art_collection_) {
            names.push_back(pair.first);
        }
        return names;
    }

    static void add_custom_art(const std::string& name, const std::vector<std::string>& art) {
        art_collection_[name] = art;
    }

    static std::vector<std::string> load_custom_art_from_file(const std::string& file_path) {
        std::vector<std::string> art;
        std::ifstream file(file_path);
        std::string line;

        while (std::getline(file, line)) {
            art.push_back(line);
        }

        return art;
    }

    static bool save_custom_art_to_file(const std::string& name, const std::string& file_path) {
        auto it = art_collection_.find(name);
        if (it == art_collection_.end()) return false;

        std::ofstream file(file_path);
        if (!file) return false;

        for (const auto& line : it->second) {
            file << line << "\n";
        }

        return true;
    }

    static void create_art_from_text(const std::string& text, const std::string& name) {
        std::vector<std::string> art;

        std::string border(text.length() + 4, '=');
        art.push_back("+" + border + "+");
        art.push_back("|  " + text + "  |");
        art.push_back("+" + border + "+");

        art_collection_[name] = art;
    }
};

}