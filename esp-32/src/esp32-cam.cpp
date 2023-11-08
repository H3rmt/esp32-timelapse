#include <Arduino.h>
#include <esp_camera.h>
#include <soc/soc.h>
#include <soc/rtc_cntl_reg.h>
#include <driver/rtc_io.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <esp_http_client.h>

#include "esp_defs.h"
#include "defs.h"

bool connectWIFI();
int incCounter();
void sleep();
camera_config_t configCam();

void setup()
{
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // CAUTION - We'll disable the brownout detection
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);

  camera_config_t config = configCam();

  // Disable any hold on pin 4 that was placed before ESP32 went to sleep
  // rtc_gpio_hold_dis(flashPin);
  // Use PWM channel 7 to control the white on-board LED (flash) connected to GPIO 4
  ledcSetup(7, 5000, 8);
  ledcAttachPin(flashPin, 7);

  // Short pause helps to ensure the I2C interface has initialised properly before attempting to detect the camera
  delay(250);

  esp_err_t err = esp_camera_init(&config);
  if (err != 0)
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    Serial.println("going to sleep early");
    sleep();
    return;
  }

  camera_fb_t *fbget = NULL;

  ledcWrite(7, flashPower);
  delay(startDelay);

  fbget = esp_camera_fb_get();
  ledcWrite(7, 0);

  if (!fbget)
  {
    Serial.println("Camera capture failed");
    Serial.println("going to sleep early");
    sleep();
    return;
  }
  Serial.println("Camera capture success");

  int pictureNumber = incCounter();

  if (!connectWIFI())
  {
    Serial.println("WIFI connection failed");
    Serial.println("going to sleep early");
    sleep();
    return;
  }
  Serial.println("WIFI connection success");

  delay(2000);

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
  esp_http_client_set_post_field(http_client, (const char *)fbget->buf, fbget->len);

  int connAttempts2 = 0;
  while (connAttempts2 < 9)
  {
    esp_err_t errr = esp_http_client_perform(http_client);
    Serial.print("status_code: ");
    Serial.print(errr);
    Serial.print(" - ");
    Serial.println(esp_http_client_get_status_code(http_client));

    if (esp_http_client_get_status_code(http_client) == 200)
    {
      Serial.println();
      break;
    }

    connAttempts2++;
    if (connAttempts2 % 3 == 0)
    {
      // reconnect WIFI
      connectWIFI();
    }
    delay(15000);
  }

  esp_http_client_cleanup(http_client);
  esp_camera_fb_return(fbget);

  Serial.println("going to sleep");

  sleep();
}

bool connectWIFI()
{
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int connAttempts = 0;
  Serial.println("Starting WIFI connection");
  while (connAttempts < 20)
  {
    Serial.print(".");

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println();
      break;
    }

    connAttempts++;
    delay(15000);
  }

  if (!WiFi.isConnected())
  {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    return false;
  }
  Serial.print("Signal Level: ");
  Serial.println(WiFi.RSSI());
  Serial.println("-50: excelent; -60: very good; -70: good; -80: low; -90: Very low");
  return true;
}

int incCounter()
{
  int pictureNumber = 0;

  EEPROM.begin(4);
  EEPROM.get(0, pictureNumber);
  pictureNumber += 1;
  EEPROM.put(0, pictureNumber);
  EEPROM.commit();

  Serial.print("pictureNumber: ");
  Serial.println(pictureNumber);

  return pictureNumber;
}

camera_config_t configCam()
{
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.fb_location = CAMERA_FB_IN_DRAM;
  config.frame_size = FRAMESIZE_SVGA;
  config.jpeg_quality = 8;
  config.fb_count = 1;

  return config;
}

void sleep()
{
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  // IMPORTANT - we define pin mode for the trigger pin at the end of setup, because most pins on the ESP32-CAM
  // have dual functions, and may have previously been used by the camera or SD card access. So we overwrite them here
  pinMode(triggerPin, INPUT_PULLDOWN);
  // Ensure the flash stays off while we sleep
  // rtc_gpio_hold_en(flashPin);
  // Turn off the LED
  digitalWrite(ledPin, HIGH);
  delay(1000);
  // Use this to wakeup when trigger pin goes HIGH
  // esp_sleep_enable_ext0_wakeup(triggerPin, 1);
  // Use this to wakeup when trigger pin goes LOW
  esp_sleep_enable_ext0_wakeup(triggerPin, 0);
  esp_deep_sleep_start();
}

void loop()
{
}