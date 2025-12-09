#include <WiFi.h>
#include <ArduinoOTA.h>
#include <esp_task_wdt.h>

void task(void *optionalArgs)
{
    Serial.begin(115200);
    WiFiClass::setHostname("esp32-3d-cam");
    WiFi.begin("...", "...");
    WiFi.setSleep(false);
    while (true) {
        if (WiFi.status() == WL_CONNECTED) {
            break;
        }
        Serial.print(".");
        delay(250);
    }
    Serial.println();
    const IPAddress ip = WiFi.localIP();
    Serial.printf("IP: %s, hostname: %s\r\n", ip.toString().c_str(), WiFiClass::getHostname());
    ArduinoOTA.onStart([] {
      Serial.println("Start OTA update");
    });
    ArduinoOTA.onError([](const ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
          Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
          Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
          Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
          Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
          Serial.println("End Failed");
      }
    });
    ArduinoOTA.onProgress([](const unsigned int progress, const unsigned int total) {
      Serial.printf("Progress: %u%%\r\n", progress / (total / 100));
    });
    ArduinoOTA.setPassword("esp-32-upload");
    ArduinoOTA.setHostname(WiFiClass::getHostname());
    ArduinoOTA.setMdnsEnabled(true);
    ArduinoOTA.begin();

    esp_task_wdt_add(nullptr);
    while (true)
    {
        delay(10);
        esp_task_wdt_reset();
        ArduinoOTA.handle();
    }
}

void setup() {
    xTaskCreatePinnedToCore(task, "task", 10000, nullptr, 1, nullptr, 0);
}


void loop() {
}