#pragma once

#include <Arduino.h>

void initEEPROM();
void resetEEPROM();
int incCounter();
int getCurrentCounter();
void setIdent(const String &);
String getIdent();