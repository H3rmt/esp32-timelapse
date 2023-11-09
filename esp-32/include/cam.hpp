#pragma once

#include <esp_camera.h>

camera_config_t configCam();

void initFlash();
void setFlash(uint32_t);