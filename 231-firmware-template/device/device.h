#pragma once
#define DEVICE_NAME "es-firmware-template"
#define FIRMWARE_VERSION "2.3.1"
#define DEVICE_PROJECT "231-firmware-template"
#define DEVICE_REPO "https://github.com/dark516/es-student"
#ifndef DEVICE_BOARD
#define DEVICE_BOARD "unknown"
#endif
void device_info(void);
