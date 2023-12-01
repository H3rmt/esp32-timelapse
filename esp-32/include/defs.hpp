#pragma once

#define WIFI_SSID "FRITZ!Box 8192"
#define WIFI_PASSWORD "0882-5403-2419-2315-3774-9376-6483-8657-3837"

#define ledPin GPIO_NUM_33
#define flashPin GPIO_NUM_4
#define triggerPin GPIO_NUM_13

#define flashPower 200
#define startDelay 1500
#define wifiDelay 2500

#define serverHost "192.168.187.32"
#define serverPort 8080
#define serverSecret "f348ahj235a124baa1a23"


#define ERROR_OK 1

#define ERROR_SD_INIT_FAILED 2
#define ERROR_WIFI_FAILED 3
#define ERROR_SEND_START_FAILED 4
#define ERROR_CREATE_FOLDER_FAILED 5
// 750 ms blink 
#define ERROR_CAM_INIT_FAILED 6 // 2
#define ERROR_CAM_GET_FAILED 7 // 3
#define ERROR_SAVE_IMG_FAILED 8 // 4
#define ERROR_SEND_FINISH_FAILED 9 // 5