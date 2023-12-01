#include <Arduino.h>
#include <esp_camera.h>
#include <driver/rtc_io.h>
#include <WiFi.h>

#define USESD
// #define USEFLASH

#include "cam.hpp"
#include "eeprom.hpp"
#include "defs.hpp"
#include "net.hpp"
#include "sleep.hpp"
#include "wifi.hpp"

#ifdef USESD
#include "file.hpp"
#endif

void errorLED(int code);

void initWiFi();

void setup()
{
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  initEEPROM();

  // CAUTION - We'll disable the brownout detection
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);

#ifdef USESD
  if (!openSD())
  {
    Serial.println("SD open failed");
    Serial.println("going to sleep early");
    errorLED(ERROR_SD_INIT_FAILED);
    sleep();
    return;
  }
  Serial.println("opened SD");
#endif

  String ident;
  if (getCurrentCounter() == 0)
  // if (getCurrentCounter() == 8)
  {
    Serial.println("START detected");

    initWiFi();
    if (!sendStart(ident))
    {
      Serial.println("START failed");
      Serial.println("going to sleep early");
      errorLED(ERROR_SEND_START_FAILED);
      sleep();
      return;
    }
    Serial.println("START success");

#ifdef USESD
    if (!createFolder("/" + ident))
    {
      Serial.println("createFolder failed");
      Serial.println("going to sleep early");
      errorLED(ERROR_CREATE_FOLDER_FAILED);
      sleep();
      return;
    }
#endif

    setIdent(ident);
  }
  else
  {
    ident = getIdent();
  }

  Serial.println("Identifier: " + ident);

  int pictureNumber = incCounter();
  Serial.print("pictureNumber: ");
  Serial.println(pictureNumber);

  camera_config_t config = configCam();

#ifdef USEFLASH
  initFlash();
#endif
  // Disable any hold on pin 4 that was placed before ESP32 went to sleep
  rtc_gpio_hold_dis(flashPin);

  // Short pause helps to ensure the I2C interface has initialised properly before attempting to detect the camera
  delay(150);

  esp_err_t err = esp_camera_init(&config);
  if (err != 0)
  {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    Serial.println("going to sleep early");
    errorLED(ERROR_CAM_INIT_FAILED);
    sleep();
    return;
  }

  // configSensor();

#ifdef USEFLASH
  setFlash(flashPower);
  Serial.println("started Flash");
#endif

  delay(startDelay);
  Serial.println("taking Photo");
  camera_fb_t *fbget = esp_camera_fb_get();
#ifdef USEFLASH
  setFlash(0);
#endif
  if (!fbget)
  {
    Serial.println("Camera capture failed");
    Serial.println("going to sleep early");
    errorLED(ERROR_CAM_GET_FAILED);
    sleep();
    return;
  }
  Serial.println("Camera capture success");

  // check if print stopped
  delay(startDelay * 3);
  int finish = digitalRead(triggerPin);

#ifdef USESD
  String path = "/" + ident + "/" + pictureNumber + ".jpg";

  if (!saveImg(fbget, path))
  {
    Serial.println("Saving img to SD failed");
    Serial.println("going to sleep early");
    errorLED(ERROR_SAVE_IMG_FAILED);
    sleep();
    return;
  }
#else
  initWiFi();
  if (!sendPic(String(fbget->buf, fbget->len), String(pictureNumber), ident))
  {
    Serial.println("Saving Image failed");
    Serial.println("going to sleep early");
    errorLED(ERROR_SAVE_IMG_FAILED);
    sleep();
    return;
  }
  Serial.println("SEND success");
#endif

  if (finish) // IF LOW
  {
    Serial.println("DETECTED FINISH");
    initWiFi();
#ifdef USESD
    iterateFolder("/" + ident, [](String name, String content, String ident)
                  { 
                    name.replace(".jpg", "");
                    if (!sendPic(content, name, ident))
                      {
                        Serial.println("PORT failed");
                        Serial.println("SKIP");
                        return;
                      }
                      Serial.println("SEND success"); });
#endif
    if (!sendFinish(pictureNumber + 1, ident))
    {
      Serial.println("FINISH failed");
      Serial.println("going to sleep early");
      errorLED(ERROR_SEND_FINISH_FAILED);
      sleep();
      return;
    }
    Serial.println("FINISH success");
    reset();

    delay(10000); // wait for printer to move away
    Serial.println("Going to Sleep");
    errorLED(ERROR_OK);
    sleep();
    return;
  }

  Serial.println("Going to Sleep");
  errorLED(ERROR_OK);
  sleep();
  return;
}

void loop()
{
}

void errorLED(int code)
{
  Serial.print("Error Code:");
  Serial.println(code);
  if (code <= 5)
    for (size_t i = 0; i < code; i++)
    {
      digitalWrite(ledPin, HIGH);
      delay(500);
      digitalWrite(ledPin, LOW);
      delay(500);
    }
  else
    for (size_t i = 0; i < code - 4; i++)
    {
      digitalWrite(ledPin, HIGH);
      delay(1000);
      digitalWrite(ledPin, LOW);
      delay(1000);
    }
}

void initWiFi()
{
  Serial.println("Starting WIFI connection");
  if (!connectWIFI())
  {
    Serial.println("WIFI connection failed");
    Serial.println("going to sleep early");
    errorLED(ERROR_WIFI_FAILED);
    sleep();
    return;
  }
  Serial.println("WIFI connection success");
  Serial.print("Signal Level: ");
  Serial.print(WiFi.RSSI());
  Serial.println("  [-50: excelent; -60: very good; -70: good; -80: low; -90: Very low]");

  delay(wifiDelay);
}