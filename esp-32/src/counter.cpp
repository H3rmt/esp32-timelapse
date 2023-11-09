#include <Arduino.h>
#include <EEPROM.h>

#include "counter.hpp"

int incCounter()
{
  int pictureNumber = 0;

  EEPROM.begin(4);
  EEPROM.get(0, pictureNumber);
  pictureNumber += 1;
  EEPROM.put(0, pictureNumber);
  EEPROM.commit();

  return pictureNumber;
}