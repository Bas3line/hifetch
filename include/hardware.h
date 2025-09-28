#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <glob.h>

#define MAX_DEVICES 256
#define MAX_SENSORS 128
#define HWMON_PATH "/sys/class/hwmon"
#define PCI_PATH "/sys/bus/pci/devices"
#define USB_PATH "/sys/bus/usb/devices"
#define BLOCK_PATH "/sys/class/block"

typedef struct {
    char name[128];
    char vendor[64];
    char model[128];
    char driver[64];
    float temp;
    int speed;
    float voltage;
} HardwareDevice;

typedef struct {
    char label[64];
    float value;
    float min;
    float max;
    char unit[16];
} Sensor;

typedef struct {
    HardwareDevice cpu;
    HardwareDevice gpu[8];
    HardwareDevice storage[16];
    HardwareDevice network[8];
    HardwareDevice audio[4];
    Sensor temps[MAX_SENSORS];
    Sensor fans[MAX_SENSORS];
    Sensor voltages[MAX_SENSORS];
    int gpu_count;
    int storage_count;
    int network_count;
    int audio_count;
    int temp_count;
    int fan_count;
    int voltage_count;
} HardwareInfo;

void detect_hardware(HardwareInfo *hw);
void scan_pci_devices(HardwareInfo *hw);
void scan_usb_devices(HardwareInfo *hw);
void scan_block_devices(HardwareInfo *hw);
void read_hwmon_sensors(HardwareInfo *hw);
void get_cpu_hardware(HardwareDevice *cpu);
void get_gpu_hardware(HardwareInfo *hw);
void get_network_hardware(HardwareInfo *hw);
void get_audio_hardware(HardwareInfo *hw);
char *read_sysfs_string(const char *path);
float read_sysfs_float(const char *path);
int read_sysfs_int(const char *path);