#include <Arduino.h>
#include <EEPROM.h>

#include "eeprom.hpp"

#define CounterSize sizeof(int)
#define IdentSize sizeof(char[15])

void initEEPROM()
{
  EEPROM.begin(CounterSize + IdentSize);
}

void resetEEPROM()
{
  int pictureNumber = 0;
  String ident = "00000000000000";

  EEPROM.put(0, pictureNumber);
  EEPROM.writeString(CounterSize, ident);
  EEPROM.commit();
}


int incCounter()
{
  int pictureNumber = getCurrentCounter();

  pictureNumber += 1;
  EEPROM.put(0, pictureNumber);
  EEPROM.commit();

  return pictureNumber;
}

int getCurrentCounter()
{
  int pictureNumber = 0;

  EEPROM.get(0, pictureNumber);
  return pictureNumber;
}

void setIdent(String ident)
{
  EEPROM.writeString(CounterSize, ident);
  EEPROM.commit();
}

String getIdent()
{
  char ident[15];

  EEPROM.get(CounterSize, ident);
  return String(ident);
}