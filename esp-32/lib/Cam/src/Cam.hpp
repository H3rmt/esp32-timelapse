#pragma once

#include <esp_camera.h>

namespace Cam {
    camera_config_t configCam();

    void configSensor();
}
