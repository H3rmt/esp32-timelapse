#include <Arduino.h>
#include <esp_http_client.h>

#include "defs.hpp"

#include "net.hpp"

bool sendPic(camera_fb_t *pic, int pictureNumber)
{
    esp_http_client_config_t config_client = {0};
    config_client.host = serverHost;
    config_client.port = serverPort;
    config_client.path = serverPath;
    config_client.method = HTTP_METHOD_POST;

    esp_http_client_handle_t http_client = esp_http_client_init(&config_client);

    char cstr[16];
    itoa(pictureNumber, cstr, 10);

    esp_http_client_set_header(http_client, "Content-Type", "image/jpg");
    esp_http_client_set_header(http_client, "Count", cstr);
    esp_http_client_set_header(http_client, "Authorisation", serverSecret);
    esp_http_client_set_post_field(http_client, (const char *)pic->buf, pic->len);

    int connAttempts = 0;
    Serial.println("Starting POST");
    while (connAttempts < 9)
    {
        esp_err_t errr = esp_http_client_perform(http_client);

        Serial.print("Sending POST... [code: ");
        Serial.print(errr);
        Serial.print(" - ");
        Serial.print(esp_http_client_get_status_code(http_client));
        Serial.println("]");

        if (esp_http_client_get_status_code(http_client) == 200)
        {
            esp_http_client_cleanup(http_client);
            return true;
        }

        connAttempts++;
        delay(2000);
    }

    esp_http_client_cleanup(http_client);
    return false;
}