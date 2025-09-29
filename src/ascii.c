#include "sysfetch.h"
#include <sys/ioctl.h>
#include <immintrin.h>
#include <stdint.h>

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

// CentOS/RHEL ASCII
const char* centos_ascii[] = {
    "                 \033[1;33m..\033[0m",
    "               \033[1;33m.PLTJ.\033[0m",
    "              \033[1;33m<><><><>\033[0m",
    "     \033[1;32mTHORX\033[0m    \033[1;33m><><><><\033[0m",
    "    \033[1;32mDWAYNE\033[0m   \033[1;33m><><><><>\033[0m",
    "   \033[1;32mROFLCOPTER\033[0m \033[1;33m><><><><>\033[0m",
    "  \033[1;32mLOL\033[0m\033[1;31mUNIX\033[0m \033[1;33m><><><><><\033[0m",
    " \033[1;32mNASDAQ\033[0m\033[1;31mXORZ\033[0m \033[1;33m><><><><><\033[0m",
    " \033[1;32mFEDORA\033[0m     \033[1;33m><><><><><\033[0m",
    " \033[1;32mWHATEVER\033[0m   \033[1;33m><><><><><\033[0m",
    "  \033[1;32mE\033[0m\033[1;31mL\033[0m\033[1;32mI\033[0m\033[1;31mT\033[0m\033[1;32mE\033[0m  \033[1;33m><><><><><\033[0m",
    "   \033[1;32mLINUXER\033[0m   \033[1;33m><><><><>\033[0m",
    "    \033[1;32mTUX\033[0m      \033[1;33m><><><><\033[0m",
    "     \033[1;32mgod\033[0m        \033[1;33m><><><\033[0m",
    "                 \033[1;33m><><\033[0m",
    "                  \033[1;33m><\033[0m",
    "                   \033[1;33m>\033[0m",
    "",
    ""
};

// Manjaro ASCII
const char* manjaro_ascii[] = {
    "\033[1;32m██████████████████  ████████\033[0m",
    "\033[1;32m██████████████████  ████████\033[0m",
    "\033[1;32m██████████████████  ████████\033[0m",
    "\033[1;32m██████████████████  ████████\033[0m",
    "\033[1;32m████████            ████████\033[0m",
    "\033[1;32m████████  ████████  ████████\033[0m",
    "\033[1;32m████████  ████████  ████████\033[0m",
    "\033[1;32m████████  ████████  ████████\033[0m",
    "\033[1;32m████████  ████████  ████████\033[0m",
    "\033[1;32m████████  ████████  ████████\033[0m",
    "\033[1;32m████████  ████████  ████████\033[0m",
    "\033[1;32m████████  ████████  ████████\033[0m",
    "\033[1;32m████████  ████████  ████████\033[0m",
    "\033[1;32m████████  ████████  ████████\033[0m",
    "",
    "",
    "",
    "",
    ""
};

// openSUSE ASCII
const char* opensuse_ascii[] = {
    "           \033[1;32m.;ldkO0000Okdl;.\033[0m",
    "       \033[1;32m.;d00xl:^''''''^:ok00d;.\033[0m",
    "     \033[1;32m.d00l'                'o00d.\033[0m",
    "   \033[1;32m.d0Kd'\033[0m  \033[1;31mOdxXd\033[0m    \033[1;32m'o0Xk.\033[0m",
    "  \033[1;32m.OK\033[0m\033[1;31mKKK0kOKKKKKKKKKOxOKKKKKKKk\033[0m\033[1;32m0K.\033[0m",
    " \033[1;32m,KX\033[0m \033[1;31m'xKXXXXXXXXXXXXXXXXXXXXXX0\033[0m \033[1;32mXK,\033[0m",
    " \033[1;32mKK\033[0m     \033[1;31mkXXXXXXXXXXXXXXXXXXXXKx\033[0m     \033[1;32mKK\033[0m",
    " \033[1;32mKK\033[0m      \033[1;31mxKXXXXXXXXXXXXXXXXXKx\033[0m      \033[1;32mKK\033[0m",
    " \033[1;32mKK\033[0m       \033[1;31mxKXXXXXXXXXXXXXXXKx\033[0m       \033[1;32mKK\033[0m",
    " \033[1;32mKK\033[0m        \033[1;31mxKXXXXXXXXXXXXKx\033[0m        \033[1;32mKK\033[0m",
    " \033[1;32mKK\033[0m         \033[1;31mxKXXXXXXXXXKx\033[0m         \033[1;32mKK\033[0m",
    " \033[1;32mKK\033[0m          \033[1;31mxKXXXXXXKx\033[0m          \033[1;32mKK\033[0m",
    " \033[1;32mKK\033[0m           \033[1;31mxKXXXKx\033[0m           \033[1;32mKK\033[0m",
    " \033[1;32mKK\033[0m            \033[1;31mxKKx\033[0m            \033[1;32mKK\033[0m",
    " \033[1;32mKK\033[0m\033[1;31m0\033[0m                              \033[1;31m0\033[0m\033[1;32mKK\033[0m",
    " \033[1;32m'KK.\033[0m                            \033[1;32m.KK'\033[0m",
    "  \033[1;32m'KKO.\033[0m                        \033[1;32m.OKK'\033[0m",
    "   \033[1;32m'K0k;.\033[0m                  \033[1;32m.;k0K'\033[0m",
    "    \033[1;32m'lO0Oxl:;,,,,,,,,,,;:lxO0Ol'\033[0m"
};

// Kali Linux ASCII
const char* kali_ascii[] = {
    "      \033[1;34m,.....                                       \033[0m",
    "  \033[1;34m,xNMM.                                          \033[0m",
    "\033[1;34m.OMMMMo           MMMMMP .OMMMMMMMo.  .oMMMMMM. \033[0m",
    "\033[1;34m OMMM0,      .;MMMMMMMP' ;MMMMMMMMMP .MMMMMMMMo \033[0m",
    "\033[1;34m `MMM#        ;MMMMM'     :MMMMMMM.  .MMMMMMMM  \033[0m",
    "\033[1;34m  MMMo          MMM:        MMMMP     dMMMMMMMM \033[0m",
    "\033[1;34m  OMMl          MMM.        MMMMM,    ,MMMMMMM' \033[0m",
    "\033[1;34m  ,MMM.         `MMo        `MMMMMMo, .OMMMMMP  \033[0m",
    "\033[1;34m   MMM;          MMM        OMMMMMMMMMlOMMMMMM.  \033[0m",
    "\033[1;34m   'MMM.         MMM       MMMMMP`:MMMMMMMMMMo  \033[0m",
    "\033[1;34m    'MMo         MMM      'MMMMo   .mMMMMMMMM'  \033[0m",
    "\033[1;34m     ;MM:        MMM      'MMMM:    ;MMMMMMP'   \033[0m",
    "\033[1;34m      :MM.       'MMM.     .mMMMP.    ;MMMMP'    \033[0m",
    "\033[1;34m       ;M0.       'MMM.     :MMMM:     :MM;      \033[0m",
    "\033[1;34m        .lNl.      'MMMMl..lMMMMMo      'o'       \033[0m",
    "\033[1;34m          ..          ;MMMMMMMMP'                 \033[0m",
    "\033[1;34m                        .;dOOo;.                  \033[0m",
    "",
    ""
};

// Elementary OS ASCII
const char* elementary_ascii[] = {
    "         \033[1;37meeeeeeeeeeeeeeeee\033[0m",
    "      \033[1;37meeeeeeeeeeeeeeeeeeeeee\033[0m",
    "    \033[1;37meeeeeeeeeeeeeeeeeeeeeeee\033[0m",
    "   \033[1;37meeeee\033[0m  \033[1;37meeeeeeeeeeee   eeeee\033[0m",
    "  \033[1;37meeeee   eeeeeeeeeeee    eeeee\033[0m",
    " \033[1;37meeeeee   eeeeeeeeeee    eeeeee\033[0m",
    " \033[1;37meeeeeee  eeeeeeeeeee   eeeeeee\033[0m",
    " \033[1;37meeeeeeee eeeeeeeeee   eeeeeeee\033[0m",
    " \033[1;37meeeeeeeeeeeeeeeeeeeeeeeeeeeee\033[0m",
    " \033[1;37meeeeeeeeeeeeeeeeeeeeeeeeeeeee\033[0m",
    " \033[1;37meeeeeeeeeeeeeeeeeeeeeeeeeeeee\033[0m",
    " \033[1;37meeeeeeeeeeeeeeeeeeeeeeeeeeeee\033[0m",
    " \033[1;37meeeeeeeeeeeeeeee    eeeeeeee\033[0m",
    "  \033[1;37meeeeeeeeeeeeeee     eeeeee\033[0m",
    "   \033[1;37meeeeeeeeeeeee      eeeee\033[0m",
    "    \033[1;37meeeeeeeeee       eeee\033[0m",
    "      \033[1;37meeeeee\033[0m",
    "",
    ""
};

// Pop!_OS ASCII
const char* popos_ascii[] = {
    "             \033[1;36m/////////////\033[0m",
    "         \033[1;36m/////////////////////\033[0m",
    "      \033[1;36m//////\033[0m\033[1;37m*767\033[0m\033[1;36m////////////////\033[0m",
    "    \033[1;36m//////\033[0m\033[1;37m7676767676\033[0m\033[1;36m*//////////\033[0m",
    "   \033[1;36m/////\033[0m\033[1;37m767676767676767\033[0m\033[1;36m*////////\033[0m",
    "  \033[1;36m/////\033[0m\033[1;37m76767676767676767\033[0m\033[1;36m*//////\033[0m",
    " \033[1;36m/////\033[0m\033[1;37m767676767676767676\033[0m\033[1;36m*////\033[0m",
    " \033[1;36m////\033[0m\033[1;37m76767676767676767676\033[0m\033[1;36m*///\033[0m",
    " \033[1;36m///\033[0m\033[1;37m7676767676767676767676\033[0m\033[1;36m*//\033[0m",
    " \033[1;36m//\033[0m\033[1;37m767676767676767676767676\033[0m\033[1;36m*/\033[0m",
    " \033[1;36m/\033[0m\033[1;37m76767676767676767676767676\033[0m\033[1;36m*\033[0m",
    " \033[1;37m7676767676767676767676767676\033[0m",
    " \033[1;37m7676767676767676767676767676\033[0m",
    " \033[1;37m7676767676767676767676767676\033[0m",
    " \033[1;37m7676767676767676767676767676\033[0m",
    " \033[1;37m7676767676767676767676767676\033[0m",
    "",
    "",
    ""
};

// Linux Mint ASCII
const char* mint_ascii[] = {
    " \033[1;32mMMMMMMMMMMMMMMMMMMMMMMMMMcc\033[0m",
    " \033[1;32mMMMMMMMMMMMMMMMMMMMMMMMMMMM\033[0m",
    " \033[1;32mMMMMMMMMMMMMMMMMMMMMMMMMMMMM\033[0m",
    " \033[1;32mMMMMMMMMMMMMMMMMMMMMMMMMMMMM\033[0m",
    " \033[1;32mMMMMMMMMMMMMMMMMMMMMMMMMMMMM\033[0m",
    " \033[1;32mMMMMMMMMMMM\033[0m\033[1;37mMMMMMMMMMMMMM\033[0m",
    " \033[1;32mMMMMMMMMMMM\033[0m\033[1;37mMMMMMMMMMMMMM\033[0m",
    " \033[1;32mMMMMMMMMMMM\033[0m\033[1;37mMMMMMMMMMMMMM\033[0m",
    " \033[1;32mMMMMMMMMMMM\033[0m\033[1;37mMMMMMMMMMMMMM\033[0m",
    " \033[1;32mMMMMMMMMMMM\033[0m\033[1;37mMMMMMMMMMMMMM\033[0m",
    " \033[1;32mMMMMMMMMMMM\033[0m\033[1;37mMMMMMMMMMMMMM\033[0m",
    " \033[1;32mMMMMMMMMMMM\033[0m\033[1;37mMMMMMMMMMMMMM\033[0m",
    " \033[1;32mMMMMMMMMMMM\033[0m\033[1;37mMMMMMMMMMMMMM\033[0m",
    " \033[1;32mMMMMMMMMMMM\033[0m\033[1;37mMMMMMMMMMMMMM\033[0m",
    " \033[1;32mMMMMMMMMMMMMMMMMMMMMMMMMMMM\033[0m",
    " \033[1;32mMMMMMMMMMMMMMMMMMMMMMMMMMMM\033[0m",
    " \033[1;32mMMMMMMMMMMMMMMMMMMMMMMMMMM\033[0m",
    "",
    ""
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
    } else if (strstr(distro_name, "Manjaro") || strstr(distro_name, "manjaro")) {
        return manjaro_ascii;
    } else if (strstr(distro_name, "openSUSE") || strstr(distro_name, "SUSE") || strstr(distro_name, "suse")) {
        return opensuse_ascii;
    } else if (strstr(distro_name, "CentOS") || strstr(distro_name, "centos") ||
               strstr(distro_name, "RHEL") || strstr(distro_name, "Red Hat")) {
        return centos_ascii;
    } else if (strstr(distro_name, "Kali") || strstr(distro_name, "kali")) {
        return kali_ascii;
    } else if (strstr(distro_name, "elementary") || strstr(distro_name, "Elementary")) {
        return elementary_ascii;
    } else if (strstr(distro_name, "Pop") || strstr(distro_name, "pop")) {
        return popos_ascii;
    } else if (strstr(distro_name, "Mint") || strstr(distro_name, "mint")) {
        return mint_ascii;
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

static inline void fast_memset_dashes(char *separator, int len) {
    if (len >= 8) {
        uint64_t dash_pattern = 0x2d2d2d2d2d2d2d2dULL;
        int simd_len = len & ~7;
        for (int i = 0; i < simd_len; i += 8) {
            *((uint64_t*)(separator + i)) = dash_pattern;
        }
        for (int i = simd_len; i < len; i++) {
            separator[i] = '-';
        }
    } else {
        for (int i = 0; i < len; i++) {
            separator[i] = '-';
        }
    }
}

static int get_display_width(const char *str) {
    int width = 0;
    int in_escape = 0;

    for (const char *p = str; *p; p++) {
        if (*p == '\033') {
            in_escape = 1;
        } else if (in_escape && *p == 'm') {
            in_escape = 0;
        } else if (!in_escape) {
            width++;
        }
    }
    return width;
}

static inline void get_host_info(char *host_buffer, size_t size) {
    char buffer[256];
    if (read_file_fast("/sys/class/dmi/id/product_name", buffer, sizeof(buffer))) {
        strncpy(host_buffer, buffer, size - 1);
        host_buffer[size - 1] = '\0';
    } else {
        strncpy(host_buffer, "Unknown Host", size - 1);
        host_buffer[size - 1] = '\0';
    }
}

void display_system_info(const SystemInfo *info) {
    set_ascii_for_distro(info->os_name);

    if (!current_ascii) {
        current_ascii = arch_ascii;
    }

    struct winsize w;
    int terminal_width = 80;
    if (ioctl(0, TIOCGWINSZ, &w) == 0 && w.ws_col > 0) {
        terminal_width = w.ws_col;
    }

    char formatted_info[32][512];
    int info_count = 0;

    int name_len = strlen(info->username) + strlen(info->hostname) + 1;
    char separator[128];
    fast_memset_dashes(separator, name_len < 127 ? name_len : 127);
    separator[name_len < 127 ? name_len : 127] = '\0';

    char host_info[256];
    get_host_info(host_info, sizeof(host_info));

    snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "");
    snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34m%s\033[0m@\033[1;34m%s\033[0m", info->username, info->hostname);
    snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34m%s\033[0m", separator);
    snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34mOS:\033[0m %s", info->os_name);
    snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34mHost:\033[0m %s", host_info);
    snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34mKernel:\033[0m %s", info->kernel);
    snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34mUptime:\033[0m %s", info->uptime);
    snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34mPackages:\033[0m %s", info->packages);
    snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34mShell:\033[0m %s", info->shell);
    snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34mDisplay:\033[0m %s", info->display);
    snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34mDE:\033[0m %s", info->de_version);
    snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34mWM:\033[0m %s", info->wm);
    if (strcmp(info->wm_theme, "Unknown") != 0) {
        snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34mWM Theme:\033[0m %s", info->wm_theme);
    }
    if (strcmp(info->gtk_theme, "Unknown") != 0 && strcmp(info->qt_theme, "Unknown") != 0) {
        snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34mTheme:\033[0m %s [Qt], %s [GTK2], %s [GTK3]", info->qt_theme, info->gtk_theme, info->gtk_theme);
    } else if (strcmp(info->gtk_theme, "Unknown") != 0) {
        snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34mTheme:\033[0m %s [GTK2/3/4]", info->gtk_theme);
    }
    if (strcmp(info->icon_theme, "Unknown") != 0) {
        snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34mIcons:\033[0m %s [Qt], %s [GTK2/3/4]", info->icon_theme, info->icon_theme);
    }
    if (strcmp(info->font, "Unknown") != 0) {
        snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34mFont:\033[0m %s [Qt], %s [GTK2/3/4]", info->font, info->font);
    }
    if (strcmp(info->cursor_theme, "Unknown") != 0) {
        snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34mCursor:\033[0m %s", info->cursor_theme);
    }
    snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34mTerminal:\033[0m %s", info->terminal);
    snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34mCPU:\033[0m %s", info->cpu_model);

    for (int i = 0; i < info->gpu_count; i++) {
        if (i == 0) {
            snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34mGPU %d:\033[0m %s", i + 1, info->gpu[i]);
        } else {
            snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34mGPU %d:\033[0m %s", i + 1, info->gpu[i]);
        }
    }

    snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34mMemory:\033[0m %s", info->memory);
    if (strcmp(info->swap, "No swap") != 0) {
        snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34mSwap:\033[0m %s", info->swap);
    }
    snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34mDisk:\033[0m %s", info->disk);
    snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34mLocal IP:\033[0m %s", info->local_ip);
    snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34mBattery:\033[0m %s", info->battery);
    snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34mLocale:\033[0m %s", info->locale);

    char *colors = getenv("COLORTERM");
    if (colors) {
        snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34mColors:\033[0m %s", colors);
    } else {
        snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34mColors:\033[0m 256");
    }

    char *editor = getenv("EDITOR");
    if (editor) {
        snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34mEditor:\033[0m %s", editor);
    } else {
        snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "\033[1;34mEditor:\033[0m vim");
    }

    snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "");
    snprintf(formatted_info[info_count++], sizeof(formatted_info[0]), "");

    int max_lines = info_count > ascii_line_count ? info_count : ascii_line_count;
    int max_ascii_width = 0;

    for (int i = 0; i < ascii_line_count; i++) {
        int width = get_display_width(current_ascii[i]);
        if (width > max_ascii_width) {
            max_ascii_width = width;
        }
    }

    max_ascii_width += 2;

    for (int i = 0; i < max_lines; i++) {
        if (i < ascii_line_count && i < info_count) {
            int current_width = get_display_width(current_ascii[i]);
            int padding = max_ascii_width - current_width;
            printf("%s%*s%s\n", current_ascii[i], padding, "", formatted_info[i]);
        } else if (i < ascii_line_count) {
            printf("%s\n", current_ascii[i]);
        } else if (i < info_count) {
            printf("%*s%s\n", max_ascii_width, "", formatted_info[i]);
        }
    }
}