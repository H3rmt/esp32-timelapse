#include <Arduino.h>
#include <esp_camera.h>
#include <driver/rtc_io.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

#include <Core.hpp>
#include <Debug.hpp>
#include <EEprom.hpp>
#include <Net.hpp>
#include <Wifi.hpp>
#include <Cam.hpp>

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#include "defs.hpp"

volatile bool interrupt_flag = false;
volatile unsigned long lastPressTime = 0;

void ARDUINO_ISR_ATTR interrupt() {
    if (const unsigned long now = millis(); now - lastPressTime > interruptDebounceDelay) {
        lastPressTime = now;
        interrupt_flag = true;
        Net::abortSend();
    }
}

void setupOTA() {
    const IPAddress ip = WiFi.localIP();
    Core::printf("IP: %s, hostname: %s\r\n", ip.toString().c_str(), WiFiClass::getHostname());
    ArduinoOTA.onStart([] {
        Core::println("Start OTA update");
    });
    ArduinoOTA.onError([](const ota_error_t error) {
        Core::printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Core::println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            Core::println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            Core::println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            Core::println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            Core::println("End Failed");
        }
    });
    ArduinoOTA.setPassword(OTA_PASSWORD);
    ArduinoOTA.setHostname(WiFiClass::getHostname());
    ArduinoOTA.begin();
    Core::printf("OTA started with Password %s started\r\n", OTA_PASSWORD);
}

const camera_fb_t *takePhoto() {
    Core::setFlash(flashPower);
    delay(photoDelay);
    Core::println("taking Photo");
    const camera_fb_t *camera_fb = esp_camera_fb_get();
    delay(500);
    Core::setFlash(0);
    Core::println("Photo taken");
    return camera_fb;
}

void setup() {
    Serial.begin(115200);
    pinMode(MAGNET, INPUT_PULLUP);
    attachInterrupt(MAGNET, interrupt, RISING);
    pinMode(GPIO_NUM_33, OUTPUT); // internal LED
    digitalWrite(GPIO_NUM_33, HIGH); // OFF
    Storage::init();

    // CAUTION - We'll disable the brownout detection
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    Core::initFlash();
    if (!Wifi::connect()) {
        Core::println("WIFI connection failed");
        Debug::errorExit(ERROR_WIFI_FAILED);
    }
    Core::println("WIFI connection success");
    Core::print("Signal Level: ");
    Core::print(WiFi.RSSI());
    Core::println(" [-50: excellent; -60: very good; -70: good; -80: low; -90: Very low]");

    const camera_config_t config = Cam::configCam();
    delay(150);
    // A short pause helps to ensure the I2C interface has initialised properly before attempting to detect the camera
    if (const esp_err_t err = esp_camera_init(&config); err != 0) {
        Core::printf("Camera init failed with error 0x%x\n", err);
        Wifi::disconnect();
        Debug::errorExit(ERROR_CAM_INIT_FAILED);
    }
    Cam::configSensor();
    Core::println("Camera init success");

    // allow reset via magnet
    if (Storage::getCurrentLayerCounter() == 0 || digitalRead(MAGNET) == LOW) {
        Core::println("START detected");
        Debug::flash(40, 500);
    } else {
        Core::println("no START detected");
        String ident;
        if (!Net::sendStart(ident)) {
            Core::println("START failed");
            Debug::errorExit(ERROR_SEND_START_FAILED);
        }
        Core::print("START success. Identifier:");
        Core::println(ident);
        Storage::setIdent(ident);
    }
}

void layerPhoto() {
    Core::println("timelapse photo layer");
    const auto photo = takePhoto();
    if (!photo) {
        Core::println("Camera capture failed");
        return;
    }
    Core::println("Camera capture success");
    const auto counter = Storage::getCurrentLayerCounter();
    const auto identifier = Storage::getIdent();

    bool finish = false;
    delay(detectDelay);
    if (digitalRead(MAGNET) == LOW) {
        Core::println("DETECTED FINISH");
        Debug::flash(40, 1500);
        finish = true;
    }

    if (const auto success = Net::sendPic(String(photo->buf, photo->len), String(counter), identifier, true);
        !success) {
        Core::println("Sending Image failed");
    } else {
        Core::println("Send success");
    }
    Storage::incLayerCounter();

    if (finish) {
        if (const auto success = Net::sendFinish(counter + 1, identifier); !success) {
            Core::println("FINISH success");
        } else {
            Core::println("FINISH failed, resetting eeprom anyway");
        }
        Storage::reset();
        Wifi::disconnect();
        Debug::errorExit(0);
    }
}

void minutePhoto() {
    Core::println("timelapse photo minute");
    const auto photo = takePhoto();
    if (!photo) {
        Core::println("Camera capture failed");
        return;
    }
    Core::println("Camera capture success");
    const auto counter = Storage::getCurrentMinuteCounter();
    const auto identifier = Storage::getIdent();

    if (interrupt_flag) {
        const auto layerCounter = Storage::getCurrentLayerCounter();
        // upload photo as layer and minute instead
        if (const auto success = Net::sendPic(String(photo->buf, photo->len), String(layerCounter), identifier, true);
            !success) {
            Core::println("Sending Image failed");
        } else {
            Core::println("Send success");
        }
        Storage::incLayerCounter();
    }

    if (const auto success = Net::sendPic(String(photo->buf, photo->len), String(counter), identifier, false); !
        success) {
        Core::println("Sending Image failed");
    } else {
        Core::println("Send success");
    }
    Storage::incMinuteCounter();
}

unsigned long lastMillis = millis();

void loop() {
    if (interrupt_flag) {
        interrupt_flag = false;
        layerPhoto();
    }
    if (millis() - lastMillis >= minuteMinutesDelay * 60 * 1000) {
        lastMillis = millis();
        minutePhoto();
    }
    delay(250);
}
