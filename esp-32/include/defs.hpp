#pragma once

#define MAGNET GPIO_NUM_13

// loaded from .env
// #define WIFI_SSID ""
// #define WIFI_PASSWORD ""
// #define SITE_USER ""
// #define SITE_PASSWORD ""
// #define SERVER_HOST ""
// #define SERVER_PORT 0

// 0 - 255 flash rightness (0 - 80) makes more sense as range
#define flashPower 80

// sensor triggered time = 0; 300ms starting + 150ms + photoDelay + ~500ms for photo => time when photo is taken
// here ~3000 ms as last delay
// delay after activating flash and taking photo (ms)
#define photoDelay 1500

// sensor triggered time = 0; 300ms starting + 150ms + photoDelay + ~500ms for photo + detectDelay => time when finish is checked
// here ~8000 ms as last delay
// delay in milliseconds to wait after picture to check if printer is finished
#define detectDelay 5000

// approx number of seconds to wait between minute images
#define minuteMinutesDelay 120

// delay in ms on how long to wait after interupt was trigered to accept next interrupt
#define interruptDebounceDelay 1000 * 20

// 500 ms blink
#define ERROR_SD_INIT_FAILED 2
#define ERROR_WIFI_FAILED 3
#define ERROR_SEND_START_FAILED 4
#define ERROR_CREATE_FOLDER_FAILED 5

// 1000 ms blink
#define ERROR_CAM_INIT_FAILED 6
#define ERROR_CAM_GET_FAILED 7
#define ERROR_SAVE_IMG_FAILED 8
#define ERROR_SEND_FINISH_FAILED 9