#pragma once

#include <esp_camera.h>

bool sendPic(const String &pic, const String &pictureNumber, const String &ident);
bool sendFinish(int pictureCount, const String &ident);
bool sendStart(String &ident);