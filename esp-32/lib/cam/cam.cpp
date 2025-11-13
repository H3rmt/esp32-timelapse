#include <Arduino.h>
#include <esp_camera.h>
#include <sensor.h>

#include "defs.hpp"

#include "cam.hpp"

camera_config_t configCam() {
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
    // config.xclk_freq_hz = 10000000;
    config.pixel_format = PIXFORMAT_JPEG;

    if (psramFound()) {
        println("using psram");
        config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
        config.fb_location = CAMERA_FB_IN_PSRAM;
        config.grab_mode = CAMERA_GRAB_LATEST;
        config.jpeg_quality = 10;
        config.fb_count = 2;
    } else {
        println("not using psram");
        config.frame_size = FRAMESIZE_SVGA;
        config.fb_location = CAMERA_FB_IN_DRAM;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

    return config;
}


void configSensor() {
    sensor_t *s = esp_camera_sensor_get();
    s->set_framesize(s, FRAMESIZE_UXGA);
    s->set_quality(s, 3);
    s->set_brightness(s, -1);
    s->set_contrast(s, 1);
    s->set_saturation(s, 2);

    // s->set_wb_mode(s, 0);
    // s->set_whitebal(s, 1);
    // s->set_awb_gain(s, 1);
    //
    // s->set_gainceiling(s, GAINCEILING_64X);
    // s->set_gain_ctrl(s, 1);
    // s->set_exposure_ctrl(s, 1);
    //
    // s->set_aec2(s, 1);
    // s->set_ae_level(s, 0);
    //
    s->set_bpc(s, 0);
    s->set_wpc(s, 1);
    s->set_raw_gma(s, 0);
    s->set_lenc(s, 1);
    s->set_dcw(s, 0);
}


void initFlash() {
    // Use PWM channel 7 to control the white on-board LED (flash) connected to GPIO 4
    ledcAttach(GPIO_NUM_4, 5000, 8);
    ledcWrite(GPIO_NUM_4, 0);
}

// Set flash power (0-255)
void setFlash(const uint32_t power) {
    ledcWrite(GPIO_NUM_4, power);
}
