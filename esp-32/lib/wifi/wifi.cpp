#include <Arduino.h>
#include <WiFi.h>

#include "defs.hpp"

#include "wifi.hpp"

bool connectWIFI()
{
  print("Connecting to WIFI SSID");
  println(WIFI_SSID);

  const bool disconnect = WiFi.disconnect();
  print("WIFI disconnected: ");
  println(disconnect);
  
  WiFiClass::mode(WIFI_MODE_STA);
  println("WIFI mode set to WIFI_STA");
  const wl_status_t wifi = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  print("WIFI begin returned: ");
  println(wifi);
  WiFi.setSleep(false);
  delay(wifiStartTimeout);

  println("WIFI connection attempt started");
  int connAttempts = 0;
  while (connAttempts <= wifiConnectionAttempts)
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
  WiFiClass::mode(WIFI_OFF);
}