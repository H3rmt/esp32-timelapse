#include <Arduino.h>
#include <esp_camera.h>
#include <driver/rtc_io.h>
#include <WiFi.h>

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#include "defs.hpp"

// we can either use SD card or Flash for flash as GPIO 4 is used for both
// #define USESD
#define USEFLASH

#include "cam.hpp"
#include "eeprom.hpp"
#include "net.hpp"
#include "sleep.hpp"
#include "wifi.hpp"


#ifdef USESD
#include "file/file.hpp"
#endif

#define FLASH_SEQUENCE(power, sleep) \
  setFlash((power) / 2); \
  delay(200);               \
  setFlash(0);                \
  delay(sleep);               \
  setFlash((power) / 2); \
  delay(200);               \
  setFlash(0);

[[noreturn]] void errorLEDLoop(int code);

void errorLED(int code);

void initWiFi(bool loop);

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    pinMode(GPIO_NUM_33, OUTPUT); // internal LED
    digitalWrite(GPIO_NUM_33, HIGH); // OFF
    initEEPROM();

    // CAUTION - We'll disable the brownout detection
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    // Disable any hold on pin 4 that was placed before ESP32 went to sleep
    rtc_gpio_hold_dis(GPIO_NUM_4);
    initFlash();

    String ident;
    if (getCurrentCounter() == 0)
    {
        println("START detected");
        FLASH_SEQUENCE(40, 500);

        initWiFi(true);
        if (!sendStart(ident)) {
            println("START failed");
            println("going to sleep early");
            errorLEDLoop(ERROR_SEND_START_FAILED);
        }
        println("START success");
        setIdent(ident);
    } else {
        ident = getIdent();
    }

    print("Identifier: ");
    println(ident);

    const int pictureNumber = incCounter();
    print("pictureNumber: ");
    println(pictureNumber);

    const camera_config_t config = configCam();

    // A short pause helps to ensure the I2C interface has initialised properly before attempting to detect the camera
    delay(150);

    if (const esp_err_t err = esp_camera_init(&config); err != 0) {
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
    const camera_fb_t *camera_fb = esp_camera_fb_get();
    setFlash(0);

    if (!camera_fb) {
        println("Camera capture failed");
        println("going to sleep early");
        errorLED(ERROR_CAM_GET_FAILED);
        sleep();
        return;
    }
    println("Camera capture success");

    delay(detectDelay);
    const int finish = digitalRead(GPIO_NUM_13);

    initWiFi(false);
    if (!sendPic(String(camera_fb->buf, camera_fb->len), String(pictureNumber), ident)) {
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
        FLASH_SEQUENCE(40, 2000);

        if (!sendFinish(pictureNumber + 1, ident)) {
            println("FINISH failed");
            println("going to sleep early");
            errorLED(ERROR_SEND_FINISH_FAILED);
            sleep();
            return;
        }
        println("FINISH success /sleeping 10_000");
        resetEEPROM();

        delay(10000); // wait for the printer to move away
        println("Going to Sleep");
        sleep();
        return;
    }

    FLASH_SEQUENCE(40, 200);
    println("Going to Sleep");
    sleep();
}

void loop() {
}

void errorLED(const int code) {
    print("Error Code: ");
    println(code);
    delay(1000);

    for (size_t i = 0; i < code; i++) {
        digitalWrite(GPIO_NUM_33, LOW); // ON
        delay(250);

        digitalWrite(GPIO_NUM_33, HIGH); // OFF
        delay(250);
    }
    delay(1000);
    println("Error code indicated");
}

[[noreturn]] void errorLEDLoop(const int code) {
    print("Error Code: ");
    println(code);
    delay(1000);

    while (true) {
        for (size_t i = 0; i < code; i++) {
            digitalWrite(GPIO_NUM_33, LOW); // ON
            delay(250);

            digitalWrite(GPIO_NUM_33, HIGH); // OFF
            delay(250);
        }
        delay(2000);
        println("Error code indicated");
    }
}

void initWiFi(const bool loop) {
    println("Starting WIFI connection");
    if (!connectWIFI()) {
        println("WIFI connection failed");
        println("going to sleep early");
        if (loop) {
            errorLEDLoop(ERROR_WIFI_FAILED);
        }
        errorLED(ERROR_WIFI_FAILED);
        sleep();
        return;
    }
    println("WIFI connection success");
    print("Signal Level: ");
    print(WiFi.RSSI());
    println(" [-50: excelent; -60: very good; -70: good; -80: low; -90: Very low]");
}
