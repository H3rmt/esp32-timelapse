#pragma once

#define WIFI_SSID "FRITZ!Box 8192"
#define WIFI_PASSWORD "0882-5403-2419-2315-3774-9376-6483-8657-3837"

#define ledPin GPIO_NUM_33
#define flashPin GPIO_NUM_4
#define triggerPin GPIO_NUM_13
#define fin_triggerPin GPIO_NUM_13 // TODO

#define flashPower 200
#define startDelay 1500
#define wifiDelay 2500

#define serverHost "192.168.187.32"
#define serverPort 8080
#define serverSecret "f348ahj235a124baa1a23"

#define ntpServer "pool.ntp.org"
#define gmtOffset_sec 0
#define daylightOffset_sec 3600