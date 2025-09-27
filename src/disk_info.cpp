#include "utils.hpp"
#include <sstream>

namespace sysfetch::system {

std::vector<std::string> get_disk_info() noexcept {
    std::vector<std::string> disk_info;

    if (auto df_output = utils::execute_command("df -h --output=target,size,used,avail,pcent 2>/dev/null | grep '^/'"); df_output) {
        std::istringstream iss{*df_output};
        std::string line;

        while (std::getline(iss, line)) {
            if (line.find("/dev") != std::string::npos || line.find("/") == 0) {
                disk_info.push_back(line);
            }
        }
    }

    if (disk_info.empty()) {
        if (auto df_output = utils::execute_command("df -h | grep '^/'"); df_output) {
            std::istringstream iss{*df_output};
            std::string line;

            while (std::getline(iss, line)) {
                if (line.find("/dev") != std::string::npos || line.find("/") == 0) {
                    disk_info.push_back(line);
                }
            }
        }
    }

    return disk_info;
}

}