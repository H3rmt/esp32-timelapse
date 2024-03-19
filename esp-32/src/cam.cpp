#include <Arduino.h>
#include <esp_camera.h>
#include <sensor.h>

#include "esp_defs.hpp"
#include "defs.hpp"

#include "cam.hpp"

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
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_LATEST;

    if (psramFound())
    {
        println("using psram");
        config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
        config.jpeg_quality = 12;
        config.fb_count = 2;
    }
    else
    {
        println("not using psram");
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

    return config;
}

void configSensor()
{
    sensor_t *s = esp_camera_sensor_get();
    // Gain
    s->set_gain_ctrl(s, 1);                  // Auto-Gain Control 0 = disable , 1 = enable
    s->set_agc_gain(s, 0);                   // Manual Gain 0 to 30
    s->set_gainceiling(s, (gainceiling_t)0); // 0 to 6
    // Exposure
    s->set_exposure_ctrl(s, 1); // Auto-Exposure Control 0 = disable , 1 = enable
    s->set_aec_value(s, 300);   // Manual Exposure 0 to 1200
    // Exposure Correction
    s->set_aec2(s, 0);     // Automatic Exposure Correction 0 = disable , 1 = enable
    s->set_ae_level(s, 0); // Manual Exposure Correction -2 to 2
    // White Balance
    s->set_awb_gain(s, 1);       // Auto White Balance 0 = disable , 1 = enable
    s->set_wb_mode(s, 0);        // White Balance Mode 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
    s->set_whitebal(s, 1);       // White Balance 0 = disable , 1 = enable
    s->set_bpc(s, 0);            // Black Pixel Correction 0 = disable , 1 = enable
    s->set_wpc(s, 1);            // White Pixel Correction 0 = disable , 1 = enable
    s->set_brightness(s, 0);     // Brightness -2 to 2
    s->set_contrast(s, 0);       // Contrast -2 to 2
    s->set_saturation(s, 0);     // Saturation -2 to 2
    s->set_special_effect(s, 0); // (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
    // Additional settings
    s->set_lenc(s, 1);     // Lens correction 0 = disable , 1 = enable
    s->set_hmirror(s, 0);  // Horizontal flip image 0 = disable , 1 = enable
    s->set_vflip(s, 0);    // Vertical flip image 0 = disable , 1 = enable
    s->set_colorbar(s, 1); // Colour Testbar 0 = disable , 1 = enable
    s->set_raw_gma(s, 1);  // 0 = disable , 1 = enable
    s->set_dcw(s, 1);      // 0 = disable , 1 = enable
}

void initFlash()
{
    // Use PWM channel 7 to control the white on-board LED (flash) connected to GPIO 4
    ledcSetup(7, 5000, 8);
    ledcAttachPin(flashPin, 7);
    ledcWrite(7, 0);
}
void setFlash(uint32_t power)
{
    ledcWrite(7, power);
}