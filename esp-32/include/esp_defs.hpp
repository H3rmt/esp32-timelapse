#pragma once

// CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

/*
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
s->set_colorbar(s, 0); // Colour Testbar 0 = disable , 1 = enable
s->set_raw_gma(s, 1);  // 0 = disable , 1 = enable
s->set_dcw(s, 1);      // 0 = disable , 1 = enable
*/