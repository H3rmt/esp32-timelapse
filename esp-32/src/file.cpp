#define BOARD_HAS_1BIT_SDMMC

#include <Arduino.h>
#include <FS.h>
// #include <SD.h>
// #include <LittleFS.h>
#include <SD_MMC.h>
#include <esp_camera.h>

// #include "esp_vfs_fat.h"
#include "file.hpp"

bool openSD()
{
    // if (!SD_MMC.begin("/imgs", true, false, SDMMC_FREQ_HIGHSPEED))
    if (!SD_MMC.begin())
    // int SD_CS_PIN = 19;
    // SPI.begin(18, 36, 26, SD_CS_PIN);
    // SPI.setDataMode(SPI_MODE0);
    // if (!SD.begin(SD_CS_PIN, SPI, 4000000U, "/sd", (uint8_t)5U, FORMAT_IF_FAILED))
    // if (!SD.begin())
    {
        Serial.println("SD Card Mount Failed");
        return false;
    }
    // uint8_t cardType = LittleFS.cardType();
    // if (cardType == CARD_NONE)
    // {
    //     Serial.println("No SD Card attached");
    //     return false;
    // }

    return true;
}

bool createFolder(String path)
{
    fs::FS &fs = SD_MMC;
    Serial.println("Folder name: " + path);

    if (!fs.mkdir(path))
    {
        Serial.println("Failed to create Folder");
        return false;
    }
    Serial.println("Created folder: " + path);
    return true;
}

bool saveImg(camera_fb_t *pic, String path)
{
    fs::FS &fs = SD_MMC;
    Serial.println("Picture file name: " + path);

    File file = fs.open(path, FILE_WRITE, true);
    if (!file)
    {
        Serial.println("Failed to open file in writing mode");
        file.close();
        return false;
    }

    file.write(pic->buf, pic->len);
    file.flush();
    Serial.println("Saved file to path: " + path);

    file.close();
    return true;
}

void iterateFolder(String ident, void (*function)(String, String, String))
{
    File root = SD_MMC.open(ident);

    File file = root.openNextFile();

    while (file)
    {
        Serial.println(file.name());
        Serial.println(file.readString());
        function(String(file.name()), file.readString(), ident);
        file = root.openNextFile();
    }
}
