#include <Arduino.h>
#include <WiFi.h>

#include "defs.hpp"

#include "wifi.hpp"

bool connectWIFI()
{
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int connAttempts = 0;
  while (connAttempts < 20)
  {
    Serial.println("Connecting WIFI... ");
    delay(5000);

    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }

    connAttempts++;
  }

  wifiOff();
  return false;
}

void wifiOff() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}