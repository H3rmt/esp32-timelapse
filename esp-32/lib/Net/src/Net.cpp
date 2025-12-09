#include <Arduino.h>
#include <Core.hpp>
#include <HTTPClient.h>
#include <Wifi.hpp>
#include <Net.hpp>

bool Net::sendPic(const String &pic, const String &pictureNumber, const String &ident, const bool layer) {
    abortSendFlag = false;

    HTTPClient http;
    http.begin(SERVER_HOST, SERVER_PORT,
               String("/upload") + "?count=" + pictureNumber + "&identifier=" + ident + "&layer=" + String(layer));
    http.setAuthorization(SITE_USER, SITE_PASSWORD);
    http.setConnectTimeout(1000);
    http.setTimeout(2000);
    http.setUserAgent("ESP32-CAM");
    http.setReuse(true);

    int connAttempts = 0;
    Core::printf("Starting sendPic: %s Size: %lu to:%s:%u\n",
                 http.getLocation().c_str(),
                 static_cast<unsigned long>(pic.length()),
                 SERVER_HOST,
                 static_cast<unsigned>(SERVER_PORT)
    );
    while (connAttempts <= netRetries) {
        const int httpCode = http.POST(pic);
        Core::println("Sending... [code:" + String(httpCode) + " ] " + connAttempts + " Attempt");
        if (httpCode == HTTP_CODE_OK) {
            http.end();
            return true;
        }
        connAttempts++;
        if (abortSendFlag) {
            abortSendFlag = false;
            http.end();
            return false;
        }
        if (connAttempts == netRetries / 2) {
            Wifi::reconnect();
            Core::println("Reconnecting Wifi...");
        }
        delay(netRetryDelay);
    }
    http.end();
    return false;
}

bool Net::sendFinish(const int layerCount, const int minuteCount, const String &ident) {
    abortSendFlag = false;

    HTTPClient http;
    http.begin(SERVER_HOST, SERVER_PORT,
               String("/finish") + "?layer_count=" + layerCount + "&minute_count=" + minuteCount + "&identifier=" +
               ident);
    http.setAuthorization(SITE_USER, SITE_PASSWORD);
    http.setConnectTimeout(1000);
    http.setTimeout(2000);
    http.setUserAgent("ESP32-CAM");
    http.setReuse(true);

    int connAttempts = 0;
    Core::printf("Starting sendFinish: %sto:%s:%u\n",
                 http.getLocation().c_str(),
                 SERVER_HOST,
                 static_cast<unsigned>(SERVER_PORT)
    );
    Core::println("Starting sendFinish: " + http.getLocation());
    while (connAttempts <= netRetries) {
        const int httpCode = http.GET();
        Core::println("Sending... [code:" + String(httpCode) + " ] " + connAttempts + " Attempt");
        if (httpCode == HTTP_CODE_OK) {
            http.end();
            return true;
        }

        connAttempts++;
        if (abortSendFlag) {
            abortSendFlag = false;
            http.end();
            return false;
        }
        if (connAttempts == netRetries / 2) {
            Wifi::reconnect();
            Core::println("Reconnecting Wifi...");
        }
        delay(netRetryDelay);
    }
    http.end();
    return false;
}

bool Net::sendStart(String &ident) {
    abortSendFlag = false;

    HTTPClient http;
    http.begin(SERVER_HOST, SERVER_PORT, "/start");
    http.setAuthorization(SITE_USER, SITE_PASSWORD);
    http.setConnectTimeout(1000);
    http.setTimeout(2000);
    http.setUserAgent("ESP32-CAM");
    http.setReuse(true);

    int connAttempts = 0;
    Core::printf("Starting sendStart: %sto:%s:%u\n",
                 http.getLocation().c_str(),
                 SERVER_HOST,
                 static_cast<unsigned>(SERVER_PORT)
    );
    while (connAttempts <= netRetries) {
        const int httpCode = http.GET();
        Core::println("Sending... [code:" + String(httpCode) + " ] " + connAttempts + " Attempt");
        if (httpCode == HTTP_CODE_OK) {
            const String payload = http.getString();
            Core::println("Received: " + payload);
            ident = payload;
            http.end();
            return true;
        }

        connAttempts++;
        if (abortSendFlag) {
            abortSendFlag = false;
            http.end();
            return false;
        }
        if (connAttempts == netRetries / 2) {
            Wifi::reconnect();
            Core::println("Reconnecting Wifi...");
        }
        delay(netRetryDelay);
    }
    http.end();
    return false;
}


void Net::abortSend() {
    abortSendFlag = true;
}
