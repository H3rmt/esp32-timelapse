#pragma once

#include <esp_camera.h>

bool sendPic(String pic, String pictureNumber, String ident);
bool sendFinish(int pictureCount, String ident);
bool sendStart(String &ident);