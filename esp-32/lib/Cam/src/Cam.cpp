#include <Arduino.h>
#include <esp_camera.h>
#include <sensor.h>

#include "defs.hpp"
#include "Cam.hpp"

#include <Core.hpp>

camera_config_t Cam::configCam() {
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
    // config.xclk_freq_hz = 20000000;
    config.xclk_freq_hz = 15000000;

    config.pixel_format = PIXFORMAT_JPEG;

    if (psramFound()) {
        Core::println("using psram");
        // config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
        config.frame_size = FRAMESIZE_SXGA;
        config.fb_location = CAMERA_FB_IN_PSRAM;
        config.grab_mode = CAMERA_GRAB_LATEST;
        config.jpeg_quality = 15;
        config.fb_count = 1;
    } else {
        Core::println("not using psram");
        config.frame_size = FRAMESIZE_SVGA;
        config.fb_location = CAMERA_FB_IN_DRAM;
        config.jpeg_quality = 15;
        config.fb_count = 1;
    }

    return config;
}

void Cam::configSensor() {
    sensor_t *s = esp_camera_sensor_get();

    // s->set_framesize(s, FRAMESIZE_SXGA);
    // s->set_quality(s, 20);

    s->set_brightness(s, 2);
    s->set_contrast(s, -1);
    s->set_saturation(s, -1);
}

#ifdef old
void Cam::configSensor() {
    sensor_t *s = esp_camera_sensor_get();

    // s->set_framesize(s, FRAMESIZE_UXGA);
    // s->set_quality(s, 12);

    // Color
    // s->set_brightness(s, 0);
    s->set_brightness(s, 2);
    s->set_contrast(s, -1);
    s->set_saturation(s, -1);

    // White balance tuned for flash / LED
    s->set_whitebal(s, 1);
    s->set_awb_gain(s, 1);
    s->set_wb_mode(s, 0);

    // Exposure / gain for low light
    s->set_gain_ctrl(s, 1);
    s->set_exposure_ctrl(s, 1);
    s->set_gainceiling(s, GAINCEILING_16X);
    s->set_aec2(s, 0);
    s->set_ae_level(s, 1);

    // Corrections
    s->set_bpc(s, 1);
    s->set_wpc(s, 1);
    s->set_lenc(s, 1);
    s->set_dcw(s, 1);
}
#endif
