#pragma once

// how often to retry sending data to server
#define netRetries 7

// how long to wait after failed request
#define netTimeout 3000

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
