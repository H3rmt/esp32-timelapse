#pragma once

#include <Arduino.h>
#include <esp_camera.h>

bool openSD();
bool createFolder(String path);
bool saveImg(camera_fb_t *pic, String path);
void iterateFolder(String ident, void (*function)(String, String, String));