#include <Arduino.h>
#include <esp_camera.h>
#include <driver/rtc_io.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

#include <Core.hpp>
#include <Debug.hpp>
#include <Storage.hpp>
#include <Net.hpp>
#include <Wifi.hpp>
#include <Cam.hpp>

#include "esp_ota_ops.h"
#include <esp_task_wdt.h>

#include "defs.hpp"

bool update = false;

void setupOTA() {
    const IPAddress ip = WiFi.localIP();
    Core::printf("IP: %s, hostname: %s\r\n", ip.toString().c_str(), WiFiClass::getHostname());
    ArduinoOTA.onStart([] {
        update = true;
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
    ArduinoOTA.onProgress([](const unsigned int progress, const unsigned int total) {
        Core::printf("Progress: %u%%\r\n", progress / (total / 100));
    });
    ArduinoOTA.setPassword(OTA_PASSWORD);
    ArduinoOTA.setHostname(WiFiClass::getHostname());
    ArduinoOTA.begin();
    Core::printf("OTA started with Password %s started\r\n", OTA_PASSWORD);
}

camera_fb_t *takePhoto() {
    digitalWrite(EXTERN_FLASH, LOW);
    delay(photoDelay);
    Core::println("taking Photo");
    camera_fb_t *camera_fb = esp_camera_fb_get();
    if (!camera_fb) {
        delay(200);
        Core::println("Taking Photo failed, tying again");
        camera_fb = esp_camera_fb_get();
    }
    delay(400);
    digitalWrite(EXTERN_FLASH, HIGH);
    Core::println("Photo taken");
    return camera_fb;
}

volatile bool interrupt_flag = false;
volatile unsigned long lastPressTime = 0;

void ARDUINO_ISR_ATTR interrupt() {
    if (update)
        return;
    if (const unsigned long now = millis(); now - lastPressTime > interruptDebounceDelay) {
        lastPressTime = now;
        interrupt_flag = true;
        Net::abortSend();
    }
}


// can be from layer photo or from
void uploadLayerPhoto(const camera_fb_t *photo) {
    const auto counter = Storage::getCurrentLayerCounter();
    const auto identifier = Storage::getIdent();
    Storage::incLayerCounter();

    bool finish = false;
    delay(detectDelay);
    if (digitalRead(MAGNET) == LOW) {
        Core::println("DETECTED FINISH");
        Debug::flash(40, 1500);
        finish = true;
    } else {
        Core::println("no FINISH detected");
    }

    if (const auto success = Net::sendPic(String(photo->buf, photo->len), String(counter), identifier, true);
        !success) {
        Core::println("Sending Image failed, try again");
        if (const auto success2 = Net::sendPic(String(photo->buf, photo->len), String(counter), identifier, true);
            !success2) {
            Core::println("Sending Image failed");
        } else {
            Core::println("Send success on 2. try");
        }
    } else {
        Core::println("Send success");
    }

    if (finish) {
        const auto minuteCounter = Storage::getCurrentMinuteCounter();
        if (const auto success = Net::sendFinish(counter + 1, minuteCounter, identifier); !success) {
            Core::println("FINISH failed, resetting eeprom anyway");
            Storage::reset();
            Wifi::disconnect();
            Debug::errorExit(ERROR_SEND_FINISH_FAILED);
        }
        Core::println("FINISH success");
        Storage::reset();
        Wifi::disconnect();
        Debug::errorExit(0);
    }
}

void uploadMinutePhoto(const camera_fb_t *photo) {
    const auto counter = Storage::getCurrentMinuteCounter();
    const auto identifier = Storage::getIdent();
    Storage::incMinuteCounter();

    if (const auto success = Net::sendPic(String(photo->buf, photo->len), String(counter), identifier, false); !
        success) {
        Core::println("Sending Image failed");
    } else {
        Core::println("Send success");
    }
}

void layerPhoto() {
    Core::println("timelapse photo layer");
    const auto photo = takePhoto();
    if (!photo) {
        Core::println("Camera capture failed");
        // restart might fix it
        ESP.restart();
        return;
    }
    esp_camera_fb_return(photo);
    Core::println("Camera capture success");
    uploadLayerPhoto(photo);
}

void minutePhoto() {
    Core::println("timelapse photo minute");
    const auto photo = takePhoto();
    if (!photo) {
        Core::println("Camera capture failed");
        // restart might fix it
        ESP.restart();
        return;
    }
    esp_camera_fb_return(photo);
    Core::println("Camera capture success");
    // if layer was triggered during capture
    if (interrupt_flag) {
        uploadLayerPhoto(photo);
    }
    uploadMinutePhoto(photo);
}


unsigned long lastMillis = millis();

void pictureLoop() {
    ArduinoOTA.handle();
    if (update) {
        return;
    }
    if (interrupt_flag) {
        interrupt_flag = false;
        delay(10);
        if (digitalRead(MAGNET) == HIGH) {
            Core::println("Error - false interrupt");
            return;
        }
        layerPhoto();
    }
    if (millis() - lastMillis >= minuteMinutesDelay * 1000) {
        lastMillis = millis();
        minutePhoto();
    }
    delay(250);
}

[[noreturn]] void customSetup(void *optionalArgs) {
    Serial.begin(115200);
    pinMode(MAGNET, INPUT_PULLUP);
    pinMode(EXTERN_FLASH, OUTPUT);
    digitalWrite(EXTERN_FLASH, HIGH);
    attachInterrupt(MAGNET, interrupt, RISING);
    pinMode(GPIO_NUM_33, OUTPUT); // internal LED
    digitalWrite(GPIO_NUM_33, HIGH); // OFF
    Storage::init();

    // CAUTION - We'll disable the brownout detection
    // WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    const camera_config_t config = Cam::configCam();
    delay(150);
    // A short pause helps to ensure the I2C interface has initialised properly before attempting to detect the camera
    if (const esp_err_t err = esp_camera_init(&config); err != 0) {
        Core::printf("Camera init failed with error 0x%x\n", err);
        Wifi::disconnect();
        Debug::error(ERROR_CAM_INIT_FAILED);
        esp_ota_mark_app_invalid_rollback_and_reboot();
        ESP.restart();
    }
    Cam::configSensor();
    Core::println("Camera init success");

    Core::initFlash();
    if (!Wifi::connect()) {
        Core::println("WIFI connection failed");
        Wifi::disconnect();
        Debug::error(ERROR_WIFI_FAILED);
        esp_ota_mark_app_invalid_rollback_and_reboot();
        ESP.restart();
    }
    Core::println("WIFI connection success");
    Core::print("Signal Level: ");
    Core::print(WiFi.RSSI());
    Core::println(" [-50: excellent; -60: very good; -70: good; -80: low; -90: Very low]");
    setupOTA();
    Core::println("OTA setup success");

    // allow reset via magnet
    if (Storage::getCurrentLayerCounter() == 0 || digitalRead(MAGNET) == LOW) {
        Core::println("START detected");
        Debug::flash(40, 500);
        String ident;
        if (!Net::sendStart(ident)) {
            Core::println("START failed");
            Wifi::disconnect();
            Debug::error(ERROR_SEND_START_FAILED);
            esp_ota_mark_app_invalid_rollback_and_reboot();
            ESP.restart();
        }
        Core::print("START success. Identifier:");
        Core::println(ident);
        Storage::reset();
        Storage::setIdent(ident);
    } else {
        Core::println("no START detected");
    }

    Core::print("minute counter: ");
    Core::print(Storage::getCurrentMinuteCounter());
    Core::print("  layer counter:");
    Core::print(Storage::getCurrentLayerCounter());
    Core::print(" identifier:");
    Core::println(Storage::getIdent());
    delay(500);
    Core::println("Starting loop");

    esp_ota_mark_app_valid_cancel_rollback();
    while (true) {
        pictureLoop();
    }
}

void setup() {
    xTaskCreatePinnedToCore(customSetup, "loop", 10000, nullptr, 1, nullptr, 0);
}

void loop() {
}
