#pragma once

//  loaded from .env
extern const char *WIFI_SSID;
extern const char *WIFI_PASSWORD;
extern const char *SITE_USER;
extern const char *SITE_PASSWORD;

extern const char *SERVER_HOST;
extern const int SERVER_PORT;

#define ledPin GPIO_NUM_33
#define flashPin GPIO_NUM_4
// active on pulling low
#define triggerPin GPIO_NUM_13

#define flashPower 180
#define photoDelay 2000
#define detectDelay 5000
#define wifiDelay 1000
#define wifiTimeout 4000
#define wifiStartTimeout 2000
#define netTimeout 5000
#define netRetries 7

#define ERROR_OK 1

// 500 ms blink
#define ERROR_SD_INIT_FAILED 2
#define ERROR_WIFI_FAILED 3
#define ERROR_SEND_START_FAILED 4
#define ERROR_CREATE_FOLDER_FAILED 5
// 1000 ms blink
#define ERROR_CAM_INIT_FAILED 6    // 2
#define ERROR_CAM_GET_FAILED 7     // 3
#define ERROR_SAVE_IMG_FAILED 8    // 4
#define ERROR_SEND_FINISH_FAILED 9 // 5

#define debug

#ifdef debug
#define print(d) Serial.print(d);
#define println(d) Serial.println(d);
#define printf(d, ...) Serial.printf(d, __VA_ARGS__);
#else
#define print(d) \
    do           \
    {            \
    } while (0)
#define println(d) \
    do             \
    {              \
    } while (0)
#define printf(d, ...) \
    do                 \
    {                  \
    } while (0)
#endif
