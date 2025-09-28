#include "sysfetch.h"

int main(void) {
    SystemInfo info = {0};
    get_system_info(&info);
    display_system_info(&info);
    return 0;
}