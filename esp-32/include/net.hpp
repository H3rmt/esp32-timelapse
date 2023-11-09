#pragma once

#include <esp_camera.h>

bool sendPic(camera_fb_t*, int);
bool sendFinish(int);
bool sendStart();