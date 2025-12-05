#include <Arduino.h>
#include <Core.hpp>

#include "Debug.hpp"

void Debug::flash(const int power, const int sleep) {
    for (int i = 0; i < 4; i++) {
        Core::setFlash(power);
        delay(200);
        Core::setFlash(0);
        delay(sleep);
    }
}

[[noreturn]] void Debug::errorExit(const int code) {
    Core::printf("Error code %d\r\n", code);
    while (true) {
        for (int i = 0; i < code; i++) {
            digitalWrite(GPIO_NUM_33, LOW);
            delay(200);
            digitalWrite(GPIO_NUM_33, HIGH);
            delay(200);
        }
        delay(2000);
    }
}

void Debug::error(const int code) {
    Core::printf("Error code %d\r\n", code);
    for (size_t i = 0; i < code; i++) {
        digitalWrite(GPIO_NUM_33, LOW);
        delay(200);
        digitalWrite(GPIO_NUM_33, HIGH);
        delay(200);
    }
    delay(2000);
    Core::println("Error code indicated");
}
