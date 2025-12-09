#include <Arduino.h>
#include <WiFi.h>

#include "Wifi.hpp"

#include <Core.hpp>

bool Wifi::connect() {
    Core::print("Setting Hostname: ");
    Core::println(ESP_HOSTNAME);
    WiFiClass::mode(WIFI_MODE_STA);
    WiFiClass::setHostname(ESP_HOSTNAME);
    Core::print("Connecting to WIFI SSID: ");
    Core::println(WIFI_SSID);
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(startTimeout);

    Core::println("WIFI connection attempt started");
    int connAttempts = 0;
    while (connAttempts <= connectionAttempts) {
        Core::print("Connecting WIFI... (");
        Core::print(WiFi.status());
        Core::print(") Attempt");
        Core::println(connAttempts);
        if (WiFi.status() == WL_CONNECTED) {
            return true;
        }

        // reconnect every 20th attempt => 5 seconds
        if (connAttempts % 20 == 0) {
            disconnect();
            delay(startTimeout);
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
            delay(startTimeout);
        }

        delay(timeout);
        connAttempts++;
    }
    return false;
}

void Wifi::disconnect() {
    WiFi.disconnect(true);
    WiFiClass::mode(WIFI_OFF);
}

void Wifi::reconnect() {
    WiFi.reconnect();
}
