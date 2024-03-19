#include <Arduino.h>
#include <WiFi.h>

#include "defs.hpp"

#include "wifi.hpp"

bool connectWIFI()
{
  // WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  delay(wifiStartTimeout);

  int connAttempts = 0;
  while (connAttempts <= 24)
  {
    print("Connecting WIFI... (");
    print(WiFi.status());
    print(") Attempt");
    println(connAttempts);

    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }

    // reconnect every 7th attempt
    if (connAttempts % 7 == 0)
    {
      wifiOff();
      delay(wifiStartTimeout);
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      delay(wifiStartTimeout);
    }

    delay(wifiTimeout);
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