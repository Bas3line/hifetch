#include "sysfetch.h"

// Arch Linux ASCII
const char* arch_ascii[] = {
    "                  \033[1;34m-`\033[0m",
    "                 \033[1;34m.o+`\033[0m",
    "                \033[1;34m`ooo/\033[0m",
    "               \033[1;34m`+oooo:\033[0m",
    "              \033[1;34m`+oooooo:\033[0m",
    "              \033[1;34m-+oooooo+:\033[0m",
    "            \033[1;34m`/:-:++oooo+:\033[0m",
    "           \033[1;34m`/++++/+++++++:\033[0m",
    "          \033[1;34m`/++++++++++++++:\033[0m",
    "         \033[1;34m`/+++ooooooooooooo/`\033[0m",
    "        \033[1;34m./ooosssso++osssssso+`\033[0m",
    "       \033[1;34m.oossssso-````/ossssss+`\033[0m",
    "      \033[1;34m-osssssso.      :ssssssso.\033[0m",
    "     \033[1;34m:osssssss/        osssso+++.\033[0m",
    "    \033[1;34m/ossssssss/        +ssssooo/-\033[0m",
    "  \033[1;34m`/ossssso+/:-        -:/+osssso+-\033[0m",
    " \033[1;34m`+sso+:-`                 `.-/+oso:\033[0m",
    "\033[1;34m`++:.                           `-/+/\033[0m",
    "\033[1;34m.`                                 `/\033[0m"
};

// Ubuntu ASCII
const char* ubuntu_ascii[] = {
    "            \033[1;31m.-/+oossssoo+/-.\033[0m",
    "        \033[1;31m`:+ssssssssssssssssss+:`\033[0m",
    "      \033[1;31m-+ssssssssssssssssssyyssss+-\033[0m",
    "    \033[1;31m.ossssssssssssssssssdMMMNysssso.\033[0m",
    "   \033[1;31m/ssssssssssshdmmNNmmyNMMMMhssssss/\033[0m",
    "  \033[1;31m+ssssssssshmydMMMMMMMNddddyssssssss+\033[0m",
    " \033[1;31m/sssssssshNMMMyhhyyyyhmNMMMNhssssssss/\033[0m",
    " \033[1;31m.ssssssssdMMMNhsssssssssshNMMMdssssssss.\033[0m",
    " \033[1;31m+sssshhhyNMMNyssssssssssssyNMMMysssssss+\033[0m",
    " \033[1;31mossyNMMMNyMMhsssssssssssssshmmmhssssssso\033[0m",
    " \033[1;31mossyNMMMNyMMhsssssssssssssshmmmhssssssso\033[0m",
    " \033[1;31m+sssshhhyNMMNyssssssssssssyNMMMysssssss+\033[0m",
    " \033[1;31m.ssssssssdMMMNhsssssssssshNMMMdssssssss.\033[0m",
    " \033[1;31m/sssssssshNMMMyhhyyyyhdNMMMNhssssssss/\033[0m",
    "  \033[1;31m+ssssssssshmydMMMMMMMNddddyssssssss+\033[0m",
    "   \033[1;31m/ssssssssssshdmNNNNmyNMMMMhssssss/\033[0m",
    "    \033[1;31m.ossssssssssssssssssdMMMNysssso.\033[0m",
    "      \033[1;31m-+sssssssssssssssssyyyssss+-\033[0m",
    "        \033[1;31m`:+ssssssssssssssssss+:`\033[0m"
};

// Debian ASCII
const char* debian_ascii[] = {
    "       \033[1;31m_,met$$$$$gg.\033[0m",
    "    \033[1;31m,g$$$$$$$$$$$$$$$P.\033[0m",
    "  \033[1;31m,g$$P\"     \"\"\"Y$$.\"`.\033[0m",
    " \033[1;31m,$$P'              `$$$.\033[0m",
    "\033[1;31m',$$P       ,ggs.     `$$b:\033[0m",
    "\033[1;31m`d$$'     ,$P\"'   .    $$$\033[0m",
    " \033[1;31m$$P      d$'     ,      $$P\033[0m",
    " \033[1;31m$$:      $$.   -    ,d$$'\033[0m",
    " \033[1;31m$$;      Y$b._   _,d$P'\033[0m",
    " \033[1;31mY$$.    `.`\"Y$$$$P\"'\033[0m",
    " \033[1;31m`$$b      \"-.__\033[0m",
    "  \033[1;31m`Y$$\033[0m",
    "   \033[1;31m`Y$$.\033[0m",
    "     \033[1;31m`$$b.\033[0m",
    "       \033[1;31m`Y$$b.\033[0m",
    "          \033[1;31m`\"Y$b._\033[0m",
    "              \033[1;31m`\"\"\"\033[0m",
    "",
    ""
};

// Fedora ASCII
const char* fedora_ascii[] = {
    "             \033[1;34m.',;::::;,'.\033[0m",
    "         \033[1;34m.';;;;;looolccccccoo:\033[0m",
    "        \033[1;34m.;;;;;lllllccccccoooooooo\033[0m",
    "       \033[1;34m;;;;;lllllllccccccccoooooooo\033[0m",
    "      \033[1;34m;;;;;;lllllllcccccccccccccccccc\033[0m",
    "     \033[1;34m;;;;;;;;lllllllcccccccccccccccccc\033[0m",
    "    \033[1;34m;;;;;;;;;lllllllcccccccccccccccccc\033[0m",
    "   \033[1;34m;;;;;;;;;;lllllllcccccccccccccccccc\033[0m",
    "  \033[1;34m;;;;;;;;;;;lllllllcccccccccccccccccc\033[0m",
    " \033[1;34m;;;;;;;;;;;;;lllllllcccccccccccccccc\033[0m",
    "\033[1;34m;;;;;;;;;;;;;;lllllllcccccccccccccc\033[0m",
    "\033[1;34m;;;;;;;;;;;;;;;lllllllcccccccccccc\033[0m",
    "\033[1;34m;;;;;;;;;;;;;;;;lllllllcccccccccc\033[0m",
    "\033[1;34m;;;;;;;;;;;;;;;;;lllllllcccccccc\033[0m",
    "\033[1;34m;;;;;;;;;;;;;;;;;;lllllllcccccc\033[0m",
    "\033[1;34m;;;;;;;;;;;;;;;;;;;lllllllccc\033[0m",
    "\033[1;34m;;;;;;;;;;;;;;;;;;;;llllllc\033[0m",
    "\033[1;34m;;;;;;;;;;;;;;;;;;;;;lllll\033[0m",
    "\033[1;34m;;;;;;;;;;;;;;;;;;;;;;ll\033[0m"
};

// Generic Linux ASCII
const char* linux_ascii[] = {
    "        \033[1;37m#####\033[0m",
    "       \033[1;37m#######\033[0m",
    "       \033[1;37m##\033[0m\033[1;33mO\033[0m\033[1;37m#\033[0m\033[1;33mO\033[0m\033[1;37m##\033[0m",
    "       \033[1;37m#\033[0m\033[1;33m#####\033[0m\033[1;37m#\033[0m",
    "     \033[1;37m##\033[0m\033[1;33m##\033[0m\033[1;37m#\033[0m\033[1;33m##\033[0m\033[1;37m##\033[0m",
    "    \033[1;37m#########\033[0m\033[1;33m##\033[0m\033[1;37m#\033[0m",
    "   \033[1;37m#############\033[0m",
    "   \033[1;37m#############\033[0m",
    "   \033[1;33m##\033[0m\033[1;37m###########\033[0m\033[1;33m##\033[0m",
    "   \033[1;33m######\033[0m\033[1;37m#\033[0m\033[1;33m#\033[0m\033[1;37m#\033[0m\033[1;33m######\033[0m",
    "   \033[1;33m#################\033[0m",
    "   \033[1;33m#################\033[0m",
    "    \033[1;33m###############\033[0m",
    "     \033[1;33m#############\033[0m",
    "      \033[1;33m###########\033[0m",
    "        \033[1;33m#######\033[0m",
    "         \033[1;33m#####\033[0m",
    "",
    ""
};

const char** current_ascii = NULL;
int ascii_line_count = 19;

const char** get_distro_ascii(const char* distro_name) {
    if (strstr(distro_name, "Arch") || strstr(distro_name, "arch")) {
        return arch_ascii;
    } else if (strstr(distro_name, "Ubuntu") || strstr(distro_name, "ubuntu")) {
        return ubuntu_ascii;
    } else if (strstr(distro_name, "Debian") || strstr(distro_name, "debian")) {
        return debian_ascii;
    } else if (strstr(distro_name, "Fedora") || strstr(distro_name, "fedora")) {
        return fedora_ascii;
    } else {
        return linux_ascii;
    }
}

void set_ascii_for_distro(const char* distro_name) {
    current_ascii = get_distro_ascii(distro_name);
}

void print_ascii_art(void) {
    if (!current_ascii) {
        current_ascii = arch_ascii;
    }
    for (int i = 0; i < ascii_line_count; i++) {
        printf("%s\n", current_ascii[i]);
    }
}

void display_system_info(const SystemInfo *info) {
    // Set ASCII based on distro
    set_ascii_for_distro(info->os_name);

    if (!current_ascii) {
        current_ascii = arch_ascii;
    }

    char formatted_info[19][256];

    // Calculate username@hostname separator length
    int name_len = strlen(info->username) + strlen(info->hostname) + 1;
    char separator[64];
    for (int i = 0; i < name_len && i < 63; i++) {
        separator[i] = '-';
    }
    separator[name_len < 63 ? name_len : 63] = '\0';

    snprintf(formatted_info[0], sizeof(formatted_info[0]), "");
    snprintf(formatted_info[1], sizeof(formatted_info[1]), "\033[1;34m%s\033[0m@\033[1;34m%s\033[0m", info->username, info->hostname);
    snprintf(formatted_info[2], sizeof(formatted_info[2]), "\033[1;34m%s\033[0m", separator);
    snprintf(formatted_info[3], sizeof(formatted_info[3]), "\033[1;34mOS:\033[0m %s", info->os_name);
    snprintf(formatted_info[4], sizeof(formatted_info[4]), "\033[1;34mHost:\033[0m Victus by HP Gaming Laptop 15-fa1xxx");
    snprintf(formatted_info[5], sizeof(formatted_info[5]), "\033[1;34mKernel:\033[0m %s", info->kernel);
    snprintf(formatted_info[6], sizeof(formatted_info[6]), "\033[1;34mUptime:\033[0m %s", info->uptime);
    snprintf(formatted_info[7], sizeof(formatted_info[7]), "\033[1;34mPackages:\033[0m %s", info->packages);
    snprintf(formatted_info[8], sizeof(formatted_info[8]), "\033[1;34mShell:\033[0m %s", info->shell);
    snprintf(formatted_info[9], sizeof(formatted_info[9]), "\033[1;34mDisplay:\033[0m %s", info->display);
    snprintf(formatted_info[10], sizeof(formatted_info[10]), "\033[1;34mDE:\033[0m %s", info->de);
    snprintf(formatted_info[11], sizeof(formatted_info[11]), "\033[1;34mWM:\033[0m %s", info->wm);
    snprintf(formatted_info[12], sizeof(formatted_info[12]), "\033[1;34mTerminal:\033[0m %s", info->terminal);
    snprintf(formatted_info[13], sizeof(formatted_info[13]), "\033[1;34mCPU:\033[0m %s", info->cpu_model);
    snprintf(formatted_info[14], sizeof(formatted_info[14]), "\033[1;34mGPU:\033[0m %s", info->gpu);
    snprintf(formatted_info[15], sizeof(formatted_info[15]), "\033[1;34mMemory:\033[0m %s", info->memory);
    snprintf(formatted_info[16], sizeof(formatted_info[16]), "\033[1;34mDisk:\033[0m %s", info->disk);
    snprintf(formatted_info[17], sizeof(formatted_info[17]), "\033[1;34mLocal IP:\033[0m %s", info->local_ip);
    snprintf(formatted_info[18], sizeof(formatted_info[18]), "\033[1;34mBattery:\033[0m %s", info->battery);

    for (int i = 0; i < ascii_line_count; i++) {
        if (i < 19) {
            printf("%-50s %s\n", current_ascii[i], formatted_info[i]);
        } else {
            printf("%s\n", current_ascii[i]);
        }
    }
}