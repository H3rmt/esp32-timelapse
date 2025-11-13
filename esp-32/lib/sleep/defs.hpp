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
