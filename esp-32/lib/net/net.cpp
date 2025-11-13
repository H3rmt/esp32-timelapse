#include <Arduino.h>

#include <HTTPClient.h>
#include <esp_camera.h>

#include "defs.hpp"
#include "net.hpp"

bool sendPic(const String &pic, const String &pictureNumber, const String &ident)
{
    HTTPClient http;
    http.begin(SERVER_HOST, SERVER_PORT, String("/upload") + "?count=" + pictureNumber + "&identifier=" + ident);
    http.setAuthorization(SITE_USER, SITE_PASSWORD);
    http.setUserAgent("ESP32-CAM");
    http.setReuse(true);

    int connAttempts = 0;
    print(String("Starting sendPic: ") + http.getLocation() + " Size: " + pic.length() + " to:"  + SERVER_HOST + ":" + SERVER_PORT);
    while (connAttempts <= netRetries)
    {
        const int httpCode = http.POST(pic);
        println("Sending... [code:" + String(httpCode) + " ] " + connAttempts + " Attempt");

        if (httpCode == HTTP_CODE_OK)
        {
            http.end();
            return true;
        }

        connAttempts++;
        delay(netTimeout);
    }
    http.end();
    return false;
}

bool sendFinish(const int pictureCount, const String &ident)
{
    HTTPClient http;
    http.begin(SERVER_HOST, SERVER_PORT, String("/finish") + "?count=" + pictureCount + "&identifier=" + ident);
    http.setAuthorization(SITE_USER, SITE_PASSWORD);
    http.setUserAgent("ESP32-CAM");
    http.setReuse(true);

    int connAttempts = 0;
    println("Starting sendFinish: " + http.getLocation());
    while (connAttempts <= netRetries)
    {
        const int httpCode = http.GET();
        println("Sending... [code:" + String(httpCode) + " ] " + connAttempts + " Attempt");

        if (httpCode == HTTP_CODE_OK)
        {
            http.end();
            return true;
        }

        connAttempts++;
        delay(netTimeout);
    }
    http.end();
    return false;
}

bool sendStart(String &ident)
{
    HTTPClient http;
    http.begin(SERVER_HOST, SERVER_PORT, "/start");
    http.setAuthorization(SITE_USER, SITE_PASSWORD);
    http.setUserAgent("ESP32-CAM");
    http.setReuse(true);

    int connAttempts = 0;
    println("Starting sendStart: " + http.getLocation());
    while (connAttempts <= netRetries)
    {
        const int httpCode = http.GET();
        println("Sending... [code:" + String(httpCode) + " ] " + connAttempts + " Attempt");

        if (httpCode == HTTP_CODE_OK)
        {
            const String payload = http.getString();
            println("Received: " + payload);
            ident = payload;

            http.end();
            return true;
        }

        connAttempts++;
        delay(netTimeout);
    }
    http.end();
    return false;
}