#include <Arduino.h>
#include <esp_camera.h>
#include <soc/soc.h>
#include <soc/rtc_cntl_reg.h>
#include <driver/rtc_io.h>
#include <WiFi.h>
#include <esp_http_client.h>

#include "cam.hpp"
#include "counter.hpp"
#include "defs.hpp"
#include "net.hpp"
#include "sleep.hpp"
#include "time.hpp"
#include "wifi.hpp"

int incCounter();
void sleep();
camera_config_t configCam();

void setup()
{
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  initEEPROM();

  // CAUTION - We'll disable the brownout detection
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);

  if (getCurrentCounter() == 0)
  {
    if (!sendStart())
    {
      Serial.println("START failed");
      Serial.println("going to sleep early");
      sleep();
      return;
    }
    Serial.println("START success");
  }

  int pictureNumber = incCounter();
  Serial.print("pictureNumber: ");
  Serial.println(pictureNumber);

  camera_config_t config = configCam();

  initFlash();

  // Disable any hold on pin 4 that was placed before ESP32 went to sleep
  // rtc_gpio_hold_dis(flashPin);

  // Short pause helps to ensure the I2C interface has initialised properly before attempting to detect the camera
  delay(150);

  esp_err_t err = esp_camera_init(&config);
  if (err != 0)
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    Serial.println("going to sleep early");
    sleep();
    return;
  }

  setFlash(flashPower);
  Serial.println("started Flash");

  delay(startDelay);
  Serial.println("taking Photo");
  camera_fb_t *fbget = esp_camera_fb_get();
  setFlash(0);
  if (!fbget)
  {
    Serial.println("Camera capture failed");
    Serial.println("going to sleep early");
    sleep();
    return;
  }
  Serial.println("Camera capture success");

  Serial.println("Starting WIFI connection");
  if (!connectWIFI())
  {
    Serial.println("WIFI connection failed");
    Serial.println("going to sleep early");
    sleep();
    return;
  }
  Serial.println("WIFI connection success");
  Serial.print("Signal Level: ");
  Serial.print(WiFi.RSSI());
  Serial.println("  [-50: excelent; -60: very good; -70: good; -80: low; -90: Very low]");

  delay(wifiDelay);

  if (!sendPic(fbget, pictureNumber))
  {
    Serial.println("PORT failed");
    Serial.println("going to sleep early");
    sleep();
    return;
  }
  Serial.println("POST success");

  esp_camera_fb_return(fbget);

  if (digitalRead(fin_triggerPin))
  {
    if (!sendFinish(pictureNumber + 1))
    {
      Serial.println("FINISH failed");
      Serial.println("going to sleep early");
      sleep();
      return;
    }
    Serial.println("FINISH success");
    resetCounter();
  }

  Serial.println("going to sleep");
  sleep();
}

void loop()
{
}