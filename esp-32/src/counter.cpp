#include <Arduino.h>
#include <EEPROM.h>

#include "counter.hpp"

void initEEPROM()
{
  EEPROM.begin(4);
}

int incCounter()
{
  int pictureNumber = getCurrentCounter();

  pictureNumber += 1;
  EEPROM.put(0, pictureNumber);
  EEPROM.commit();

  return pictureNumber;
}

void resetCounter()
{
  int pictureNumber = 0;

  EEPROM.put(0, pictureNumber);
  EEPROM.commit();
}

int getCurrentCounter()
{
  int pictureNumber = 0;

  EEPROM.get(0, pictureNumber);
  return pictureNumber;
}