#include <Arduino.h>
// #include <esp_http_client.h>
#include <HTTPClient.h>
#include <esp_camera.h>

#include "defs.hpp"

#include "net.hpp"

bool sendPic(String pic, String pictureNumber, String ident)
{
    HTTPClient http;
    http.begin(serverHost, serverPort, String("/upload") + "?count=" + pictureNumber + "&identifier=" + ident);
    http.setReuse(true);

    int connAttempts = 0;
    Serial.println("Starting sendPic" + http.getLocation());
    while (connAttempts < 9)
    {
        int httpCode = http.POST(pic);
        Serial.println("Sending... [code: " + String(httpCode) + " ] {" + connAttempts + "}");

        if (httpCode == 200)
        {
            http.end();
            return true;
        }

        connAttempts++;
        delay(5000);
    }
    http.end();
    return false;
}

bool sendFinish(int pictureCount, String ident)
{
    HTTPClient http;
    http.begin(serverHost, serverPort, String("/finish") + "?count=" + pictureCount + "&identifier=" + ident);
    http.setReuse(true);

    int connAttempts = 0;
    Serial.println("Starting sendFinish" + http.getLocation());
    while (connAttempts < 9)
    {
        int httpCode = http.GET();
        Serial.println("Sending... [code: " + String(httpCode) + " ] {" + connAttempts + "}");

        if (httpCode == 200)
        {
            http.end();
            return true;
        }

        connAttempts++;
        delay(5000);
    }

    http.end();
    return false;
}

bool sendStart(String &ident)
{
    HTTPClient http;
    http.begin(serverHost, serverPort, "/start");
    http.setReuse(true);

    int connAttempts = 0;
    Serial.println("Starting sendStart" + http.getLocation());
    while (connAttempts < 9)
    {
        int httpCode = http.GET();
        Serial.println("Sending... [code: " + String(httpCode) + " ] {" + connAttempts + "}");

        if (httpCode == 200)
        {
            String payload = http.getString();
            Serial.println("Received: " + payload);
            ident = payload;

            http.end();
            return true;
        }

        connAttempts++;
        delay(5000);
    }

    http.end();
    return false;
}