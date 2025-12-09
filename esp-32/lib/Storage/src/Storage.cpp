#include <Arduino.h>
#include <EEPROM.h>

#include "Storage.hpp"

#define CounterSize sizeof(int)
#define IdentSize sizeof(char[15])
#define Counter2Size sizeof(int)

void Storage::init() {
    EEPROM.begin(CounterSize + IdentSize + Counter2Size);
}

void Storage::reset() {
    constexpr int pictureLayerNumber = 0;
    const String ident = "00000000000000";
    constexpr int pictureMinuteNumber = 0;

    EEPROM.put(0, pictureLayerNumber);
    EEPROM.writeString(CounterSize, ident);
    EEPROM.put(CounterSize + IdentSize, pictureMinuteNumber);
    EEPROM.commit();
}


int Storage::incLayerCounter() {
    int pictureNumber = getCurrentLayerCounter();

    pictureNumber += 1;
    EEPROM.put(0, pictureNumber);
    EEPROM.commit();
    return pictureNumber;
}

int Storage::incMinuteCounter() {
    int pictureNumber = getCurrentMinuteCounter();

    pictureNumber += 1;
    EEPROM.put(CounterSize + IdentSize, pictureNumber);
    EEPROM.commit();
    return pictureNumber;
}

int Storage::getCurrentLayerCounter() {
    int pictureNumber = 0;
    EEPROM.get(0, pictureNumber);
    return pictureNumber;
}

int Storage::getCurrentMinuteCounter() {
    int pictureNumber = 0;
    EEPROM.get(CounterSize + IdentSize, pictureNumber);
    return pictureNumber;
}

void Storage::setIdent(const String &ident) {
    String s = ident;
    if (s.length() > 14) {
        s = s.substring(0, 14);
    }
    EEPROM.writeString(CounterSize, s);
    EEPROM.commit();
}

String Storage::getIdent() {
    char ident[17];
    EEPROM.get(CounterSize, ident);
    return {ident};
}
