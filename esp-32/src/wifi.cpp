#include <Arduino.h>
#include <WiFi.h>

#include "defs.hpp"

#include "wifi.hpp"

bool connectWIFI()
{
  // WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  delay(2000);

  int connAttempts = 0;
  while (connAttempts < 24)
  {
    Serial.println("Connecting WIFI... {" + String(connAttempts) + "}");

    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }

    delay(5000);
    connAttempts++;
  }

  wifiOff();
  return false;
}

void wifiOff()
{
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}