#include <Arduino.h>
#include <driver/rtc_io.h>

#include "defs.hpp"
#include "wifi.hpp"

#include "sleep.hpp"


void sleep()
{
  delay(3000); // wait for 3d printer to leave trigger
  wifiOff();

  // IMPORTANT - we define pin mode for the trigger pin at the end of setup, because most pins on the ESP32-CAM
  // have dual functions, and may have previously been used by the camera or SD card access. So we overwrite them here
  pinMode(triggerPin, INPUT_PULLUP);
  // Ensure the flash stays off while we sleep
  rtc_gpio_hold_en(flashPin);
  // Turn off the LED
  digitalWrite(ledPin, HIGH);
  delay(1000);
  // Use this to wakeup when trigger pin goes HIGH
  // esp_sleep_enable_ext0_wakeup(triggerPin, 1);
  // Use this to wakeup when trigger pin goes LOW
  esp_sleep_enable_ext0_wakeup(triggerPin, 0);
  esp_deep_sleep_start();
}
