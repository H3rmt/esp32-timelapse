#pragma once

// delay after connecting to WIFI (ms)
#define wifiDelay 1000

// number of times to retry connecting to WIFI (value / 7 is reconnect times)
#define wifiConnectionAttempts 24

// how long to wait for Connection check after connecting to WIFI (ms)
#define wifiTimeout 500

// how long to wait for initial Connection check after connecting to WIFI (ms)
#define wifiStartTimeout 1000


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
