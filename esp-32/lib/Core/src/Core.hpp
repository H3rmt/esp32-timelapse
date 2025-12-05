#pragma once

#include <Arduino.h>


namespace Core {
    void print(const char str[]);

    void print(const String &s);

    void print(int value);

    void println(const char str[]);

    void println(const String &s);

    void println(int value);

    void printf(const char *format, ...);

    void setFlash(uint32_t);

    void initFlash();
}
