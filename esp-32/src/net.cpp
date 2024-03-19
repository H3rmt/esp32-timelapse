#include <Arduino.h>

#include <HTTPClient.h>
#include <esp_camera.h>

#include "defs.hpp"

#include "net.hpp"

bool sendPic(String pic, String pictureNumber, String ident)
{
    HTTPClient http;
    http.begin(SERVER_HOST, SERVER_PORT, String("/upload") + "?count=" + pictureNumber + "&identifier=" + ident);
    http.setAuthorization(SITE_USER, SITE_PASSWORD);
    http.setUserAgent("ESP32-CAM");
    http.setReuse(true);

    int connAttempts = 0;
    println("Starting sendPic: " + http.getLocation());
    while (connAttempts <= netRetries)
    {
        int httpCode = http.POST(pic);
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

bool sendFinish(int pictureCount, String ident)
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
        int httpCode = http.GET();
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
    println("Starting sendFinish: " + http.getLocation());
    while (connAttempts <= netRetries)
    {
        int httpCode = http.GET();
        println("Sending... [code:" + String(httpCode) + " ] " + connAttempts + " Attempt");

        if (httpCode == HTTP_CODE_OK)
        {
            String payload = http.getString();
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