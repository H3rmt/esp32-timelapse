#include <Arduino.h>
#include "Core.hpp"

void Core::initFlash() {
    // Use PWM channel 7 to control the white on-board LED (flash) connected to GPIO 4
    ledcAttach(GPIO_NUM_4, 5000, 8);
    ledcWrite(GPIO_NUM_4, 0);
}

// Set flash power (0-255)
void Core::setFlash(const uint32_t power) {
    ledcWrite(GPIO_NUM_4, power);
}


#ifdef CDEBUG
void Core::print(const String &s) {
    Serial.print(s);
}

void Core::print(const char str[]) {
    Serial.print(str);
}

void Core::print(const int value) {
    Serial.print(value);
}

void Core::println(const String &s) {
    Serial.println(s);
}

void Core::println(const char str[]) {
    Serial.println(str);
}

void Core::println(const int value) {
    Serial.println(value);
}

void Core::printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    Serial.vprintf(format, args);
    va_end(args);
}
#else
void Core::print(const String &s) {
}
void Core::print(const char str[]) {
}
void Core::print(int value) {
}
void Core::println(const String &s) {
}
void Core::println(const char str[]) {
}
void Core::println(int value) {
}
void Core::printf(const char *format, ...) {
}
#endif
