#include <Arduino.h>
#include <esp_camera.h>
#include <driver/rtc_io.h>
#include <WiFi.h>

// #define USESD
#define USEFLASH

#include "cam.hpp"
#include "eeprom.hpp"
#include "defs.hpp"
#include "net.hpp"
#include "sleep.hpp"
#include "wifi.hpp"

#ifdef USESD
#include "file.hpp"
#endif

#define FLASH_SEQUENCE(sleep) \
  setFlash((flashPower) / 2); \
  delay(sleep);               \
  setFlash(0);                \
  delay(sleep);               \
  setFlash((flashPower) / 2); \
  delay(sleep);               \
  setFlash(0);

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
  initFlash();
  // Disable any hold on pin 4 that was placed before ESP32 went to sleep
  rtc_gpio_hold_dis(flashPin);

  String ident;
  if (getCurrentCounter() == 0)
  // if (getCurrentCounter() == 8)
  {
    println("START detected");
    FLASH_SEQUENCE(600);

    initWiFi();
    if (!sendStart(ident))
    {
      println("START failed");
      println("going to sleep early");
      errorLED(ERROR_SEND_START_FAILED);
      sleep();
      return;
    }
    println("START success");
    setIdent(ident);
  }
  else
  {
    ident = getIdent();
  }
  print("Identifier: ");
  println(ident);

  int pictureNumber = incCounter();
  print("pictureNumber: ");
  println(pictureNumber);

  camera_config_t config = configCam();

  // Short pause helps to ensure the I2C interface has initialised properly before attempting to detect the camera
  delay(150);

  esp_err_t err = esp_camera_init(&config);
  if (err != 0)
  {
    printf("Camera init failed with error 0x%x\n", err);
    println("going to sleep early");
    errorLED(ERROR_CAM_INIT_FAILED);
    sleep();
    return;
  }

  configSensor();

  setFlash(flashPower);

  delay(photoDelay);
  println("taking Photo");
  camera_fb_t *fbget = esp_camera_fb_get();
  setFlash(0);
  
  if (!fbget)
  {
    println("Camera capture failed");
    println("going to sleep early");
    errorLED(ERROR_CAM_GET_FAILED);
    sleep();
    return;
  }
  println("Camera capture success /sleeping startDelay*3");

  delay(detectDelay);

  int finish = digitalRead(triggerPin);

  initWiFi();
  if (!sendPic(String(fbget->buf, fbget->len), String(pictureNumber), ident))
  {
    println("Sending Image failed");
    println("going to sleep early");
    errorLED(ERROR_SAVE_IMG_FAILED);
    sleep();
    return;
  }
  println("SEND success");

  if (finish == LOW) // IF LOW
  {
    println("DETECTED FINISH");
    FLASH_SEQUENCE(300);

    initWiFi();
    if (!sendFinish(pictureNumber + 1, ident))
    {
      println("FINISH failed");
      println("going to sleep early");
      errorLED(ERROR_SEND_FINISH_FAILED);
      sleep();
      return;
    }
    println("FINISH success /sleeping 10_000");
    resetEEPROM();

    delay(10000); // wait for printer to move away
    println("Going to Sleep");
    errorLED(ERROR_OK);
    sleep();
    return;
  }

  println("Going to Sleep");
  errorLED(ERROR_OK);
  sleep();
  return;
}

void loop()
{
}

void errorLED(int code)
{
  print("Error Code: ");
  println(code);

  // indicate exit code
  digitalWrite(ledPin, LOW);
  delay(1500);
  digitalWrite(ledPin, HIGH);
  delay(500);

  for (size_t i = 0; i < code; i++)
  {
    digitalWrite(ledPin, LOW); // ON
    delay(500);

    digitalWrite(ledPin, HIGH); // OFF
    delay(500);
  }
  println("Error code indicated");
}

void initWiFi()
{
  println("Starting WIFI connection");
  if (!connectWIFI())
  {
    println("WIFI connection failed");
    println("going to sleep early");
    errorLED(ERROR_WIFI_FAILED);
    sleep();
    return;
  }
  println("WIFI connection success");
  print("Signal Level: ");
  print(WiFi.RSSI());
  println(" [-50: excelent; -60: very good; -70: good; -80: low; -90: Very low]");

  delay(wifiDelay);
}