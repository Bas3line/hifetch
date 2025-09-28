#define _GNU_SOURCE
#include "hardware.h"
#include "sysfetch.h"


void get_cpu_hardware(HardwareDevice *cpu) {
    strcpy(cpu->name, "CPU");

    char buffer[4096];
    if (read_file_fast("/proc/cpuinfo", buffer, sizeof(buffer))) {
        char *line = strtok(buffer, "\n");
        while (line) {
            if (strstr(line, "vendor_id")) {
                char *colon = strchr(line, ':');
                if (colon) {
                    colon += 2;
                    strncpy(cpu->vendor, colon, sizeof(cpu->vendor) - 1);
                }
            } else if (strstr(line, "model name")) {
                char *colon = strchr(line, ':');
                if (colon) {
                    colon += 2;
                    strncpy(cpu->model, colon, sizeof(cpu->model) - 1);
                }
            }
            line = strtok(NULL, "\n");
        }
    }

    cpu->temp = read_sysfs_float("/sys/class/thermal/thermal_zone0/temp") / 1000.0f;
}

void get_gpu_hardware(HardwareInfo *hw) {
    hw->gpu_count = 0;

    glob_t glob_result;
    if (glob("/sys/class/drm/card*/device/vendor", 0, NULL, &glob_result) == 0) {
        for (size_t i = 0; i < glob_result.gl_pathc && hw->gpu_count < 8; i++) {
            char path[512];
            char *card_path = strdup(glob_result.gl_pathv[i]);
            char *device_pos = strstr(card_path, "/device");
            if (device_pos) *device_pos = '\0';

            snprintf(path, sizeof(path), "%s/device/vendor", card_path);
            char *vendor_id = read_sysfs_string(path);

            snprintf(path, sizeof(path), "%s/device/device", card_path);
            char *device_id = read_sysfs_string(path);

            if (vendor_id && device_id) {
                HardwareDevice *gpu = &hw->gpu[hw->gpu_count];
                strcpy(gpu->name, "GPU");

                if (strstr(vendor_id, "0x10de")) strcpy(gpu->vendor, "NVIDIA");
                else if (strstr(vendor_id, "0x1002")) strcpy(gpu->vendor, "AMD");
                else if (strstr(vendor_id, "0x8086")) strcpy(gpu->vendor, "Intel");
                else strcpy(gpu->vendor, "Unknown");

                snprintf(path, sizeof(path), "%s/device/subsystem_device", card_path);
                char *subsys = read_sysfs_string(path);
                if (subsys) {
                    snprintf(gpu->model, sizeof(gpu->model), "%s:%s", device_id, subsys);
                } else {
                    strcpy(gpu->model, device_id);
                }

                snprintf(path, sizeof(path), "%s/device/driver", card_path);
                char driver_link[256];
                ssize_t len = readlink(path, driver_link, sizeof(driver_link) - 1);
                if (len > 0) {
                    driver_link[len] = '\0';
                    char *driver_name = strrchr(driver_link, '/');
                    if (driver_name) {
                        strcpy(gpu->driver, driver_name + 1);
                    }
                }

                hw->gpu_count++;
            }
            free(card_path);
        }
        globfree(&glob_result);
    }
}

void get_network_hardware(HardwareInfo *hw) {
    hw->network_count = 0;
    DIR *net_dir = opendir("/sys/class/net");
    if (!net_dir) return;

    struct dirent *entry;
    while ((entry = readdir(net_dir)) && hw->network_count < 8) {
        if (entry->d_name[0] == '.') continue;

        char path[512];
        snprintf(path, sizeof(path), "/sys/class/net/%s/device", entry->d_name);

        struct stat st;
        if (stat(path, &st) == 0) {
            HardwareDevice *net = &hw->network[hw->network_count];
            strcpy(net->name, entry->d_name);

            snprintf(path, sizeof(path), "/sys/class/net/%s/device/vendor", entry->d_name);
            char *vendor_id = read_sysfs_string(path);

            snprintf(path, sizeof(path), "/sys/class/net/%s/device/device", entry->d_name);
            char *device_id = read_sysfs_string(path);

            if (vendor_id && device_id) {
                if (strstr(vendor_id, "0x8086")) strcpy(net->vendor, "Intel");
                else if (strstr(vendor_id, "0x14e4")) strcpy(net->vendor, "Broadcom");
                else if (strstr(vendor_id, "0x10ec")) strcpy(net->vendor, "Realtek");
                else strcpy(net->vendor, "Unknown");

                strcpy(net->model, device_id);
            }

            snprintf(path, sizeof(path), "/sys/class/net/%s/device/driver", entry->d_name);
            char driver_link[256];
            ssize_t len = readlink(path, driver_link, sizeof(driver_link) - 1);
            if (len > 0) {
                driver_link[len] = '\0';
                char *driver_name = strrchr(driver_link, '/');
                if (driver_name) {
                    strcpy(net->driver, driver_name + 1);
                }
            }

            snprintf(path, sizeof(path), "/sys/class/net/%s/speed", entry->d_name);
            net->speed = read_sysfs_int(path);

            hw->network_count++;
        }
    }
    closedir(net_dir);
}

void get_audio_hardware(HardwareInfo *hw) {
    hw->audio_count = 0;

    glob_t glob_result;
    if (glob("/proc/asound/card*/id", 0, NULL, &glob_result) == 0) {
        for (size_t i = 0; i < glob_result.gl_pathc && hw->audio_count < 4; i++) {
            char *card_id = read_sysfs_string(glob_result.gl_pathv[i]);
            if (card_id) {
                HardwareDevice *audio = &hw->audio[hw->audio_count];
                strcpy(audio->name, "Audio");
                strcpy(audio->model, card_id);

                char path[512];
                char *card_path = strdup(glob_result.gl_pathv[i]);
                char *id_pos = strstr(card_path, "/id");
                if (id_pos) *id_pos = '\0';

                snprintf(path, sizeof(path), "%s/../device/vendor", card_path);
                char *vendor_id = read_sysfs_string(path);
                if (vendor_id) {
                    if (strstr(vendor_id, "0x8086")) strcpy(audio->vendor, "Intel");
                    else if (strstr(vendor_id, "0x1002")) strcpy(audio->vendor, "AMD");
                    else if (strstr(vendor_id, "0x10de")) strcpy(audio->vendor, "NVIDIA");
                    else strcpy(audio->vendor, "Unknown");
                }

                free(card_path);
                hw->audio_count++;
            }
        }
        globfree(&glob_result);
    }
}

void scan_block_devices(HardwareInfo *hw) {
    hw->storage_count = 0;
    DIR *block_dir = opendir(BLOCK_PATH);
    if (!block_dir) return;

    struct dirent *entry;
    while ((entry = readdir(block_dir)) && hw->storage_count < 16) {
        if (entry->d_name[0] == '.') continue;
        if (strlen(entry->d_name) > 4) continue;

        char path[512];
        snprintf(path, sizeof(path), "%s/%s/device/model", BLOCK_PATH, entry->d_name);
        char *model = read_sysfs_string(path);

        if (model) {
            HardwareDevice *storage = &hw->storage[hw->storage_count];
            strcpy(storage->name, entry->d_name);
            strcpy(storage->model, model);

            snprintf(path, sizeof(path), "%s/%s/device/vendor", BLOCK_PATH, entry->d_name);
            char *vendor = read_sysfs_string(path);
            if (vendor) {
                strcpy(storage->vendor, vendor);
            }

            snprintf(path, sizeof(path), "%s/%s/queue/rotational", BLOCK_PATH, entry->d_name);
            int rotational = read_sysfs_int(path);
            if (rotational == 0) {
                strcat(storage->model, " (SSD)");
            } else {
                strcat(storage->model, " (HDD)");
            }

            hw->storage_count++;
        }
    }
    closedir(block_dir);
}

void read_hwmon_sensors(HardwareInfo *hw) {
    hw->temp_count = hw->fan_count = hw->voltage_count = 0;

    DIR *hwmon_dir = opendir(HWMON_PATH);
    if (!hwmon_dir) return;

    struct dirent *entry;
    while ((entry = readdir(hwmon_dir))) {
        if (entry->d_name[0] == '.') continue;

        char hwmon_path[512];
        snprintf(hwmon_path, sizeof(hwmon_path), "%s/%s", HWMON_PATH, entry->d_name);

        DIR *sensor_dir = opendir(hwmon_path);
        if (!sensor_dir) continue;

        struct dirent *sensor_entry;
        while ((sensor_entry = readdir(sensor_dir))) {
            if (sensor_entry->d_name[0] == '.') continue;

            char sensor_path[512];
            snprintf(sensor_path, sizeof(sensor_path), "%s/%s", hwmon_path, sensor_entry->d_name);

            if (strstr(sensor_entry->d_name, "temp") && strstr(sensor_entry->d_name, "_input")) {
                if (hw->temp_count < MAX_SENSORS) {
                    Sensor *temp = &hw->temps[hw->temp_count];
                    temp->value = read_sysfs_float(sensor_path) / 1000.0f;
                    strcpy(temp->unit, "°C");

                    char label_path[512];
                    snprintf(label_path, sizeof(label_path), "%s/%s_label", hwmon_path,
                            strtok(strdup(sensor_entry->d_name), "_"));
                    char *label = read_sysfs_string(label_path);
                    if (label) {
                        strcpy(temp->label, label);
                    } else {
                        snprintf(temp->label, sizeof(temp->label), "temp%d", hw->temp_count + 1);
                    }

                    hw->temp_count++;
                }
            } else if (strstr(sensor_entry->d_name, "fan") && strstr(sensor_entry->d_name, "_input")) {
                if (hw->fan_count < MAX_SENSORS) {
                    Sensor *fan = &hw->fans[hw->fan_count];
                    fan->value = read_sysfs_float(sensor_path);
                    strcpy(fan->unit, "RPM");

                    char label_path[512];
                    snprintf(label_path, sizeof(label_path), "%s/%s_label", hwmon_path,
                            strtok(strdup(sensor_entry->d_name), "_"));
                    char *label = read_sysfs_string(label_path);
                    if (label) {
                        strcpy(fan->label, label);
                    } else {
                        snprintf(fan->label, sizeof(fan->label), "fan%d", hw->fan_count + 1);
                    }

                    hw->fan_count++;
                }
            } else if (strstr(sensor_entry->d_name, "in") && strstr(sensor_entry->d_name, "_input")) {
                if (hw->voltage_count < MAX_SENSORS) {
                    Sensor *voltage = &hw->voltages[hw->voltage_count];
                    voltage->value = read_sysfs_float(sensor_path) / 1000.0f;
                    strcpy(voltage->unit, "V");

                    char label_path[512];
                    snprintf(label_path, sizeof(label_path), "%s/%s_label", hwmon_path,
                            strtok(strdup(sensor_entry->d_name), "_"));
                    char *label = read_sysfs_string(label_path);
                    if (label) {
                        strcpy(voltage->label, label);
                    } else {
                        snprintf(voltage->label, sizeof(voltage->label), "voltage%d", hw->voltage_count + 1);
                    }

                    hw->voltage_count++;
                }
            }
        }
        closedir(sensor_dir);
    }
    closedir(hwmon_dir);
}

void detect_hardware(HardwareInfo *hw) {
    memset(hw, 0, sizeof(HardwareInfo));

    get_cpu_hardware(&hw->cpu);
    get_gpu_hardware(hw);
    get_network_hardware(hw);
    get_audio_hardware(hw);
    scan_block_devices(hw);
    read_hwmon_sensors(hw);
}