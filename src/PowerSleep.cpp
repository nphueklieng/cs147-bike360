#include "PowerSleep.h"
#include "Config.h"
#include "MotionSensor.h"
#include "WifiComm.h"

#include <WiFi.h>
#include "esp_sleep.h"
#include "driver/rtc_io.h"

// do NOT deep sleep again immediately
static uint32_t disarmAwakeUntil = 0;

// state machine will react differently after a motion wake
static bool wokeFromMotion = false;

static void logWakeCause() {
  // after deep sleep the chip reboots so this only happens once in setup().
  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

  switch (cause) {
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("Wake cause: EXT0 (IMU motion interrupt)");
      wokeFromMotion = true;
      break;
    case ESP_SLEEP_WAKEUP_UNDEFINED:
      // First power-on / reset button / flash upload.
      Serial.println("Wake cause: power-on / reset");
      wokeFromMotion = false;
      break;
    default:
      Serial.print("Wake cause code: ");
      Serial.println((int)cause);
      wokeFromMotion = false;
      break;
  }
}

static void prepareImuGpioWake() {
  // INT1 triggered when acceleration exceeds threshold.
  if (!motionSensorEnableWakeInterrupt()) {
    Serial.println("PowerSleep: IMU wake interrupt failed — sleeping anyway may never wake");
  }

  // cleared any previous interrupt so INT is low
  // before we arm the ESP32, wakeup (Otherwise we wake immediately)
  motionSensorClearWakeInterrupt();
  delay(10);

  // for both light and deep sleep
  gpio_num_t wakePin = (gpio_num_t)IMU_INT_PIN;
  int level = (IMU_INT_WAKE_LEVEL == HIGH) ? 1 : 0;

  esp_err_t err = esp_sleep_enable_ext0_wakeup(wakePin, level);
  if (err != ESP_OK) {
    Serial.print("PowerSleep: ext0 enable failed: ");
    Serial.println(err);
  }
}

static void shutdownWifiForSleep() {
  // Manual esp_light_sleep_start() or deep sleep power down
  wifiShutdown(); 
  delay(50);
}

static void restoreWifiAfterLightSleep() {
  // SoftAP must be brought back so the phone can DISARM
  wifiInit();
}

void powerSleepInit() {
  logWakeCause();

  // After deep-sleep wake, the pad is left in RTC mode, so we have to 
  // return it to normal digital GPIO before we use pinMode aggain
  rtc_gpio_deinit((gpio_num_t)IMU_INT_PIN);
  pinMode(IMU_INT_PIN, INPUT);

  // give the owner a chance to connect after boot or deep wake.
  powerSleepRefreshDisarmAwakeWindow();
}

void powerSleepRefreshDisarmAwakeWindow() {
  disarmAwakeUntil = millis() + DISARM_AWAKE_MS;
  Serial.print("DISARMED awake window until ms=");
  Serial.println(disarmAwakeUntil);
}

bool powerSleepDisarmAwakeExpired() {
  return (int32_t)(millis() - disarmAwakeUntil) >= 0;
}

bool powerSleepWokeFromMotion() {
  return wokeFromMotion;
}

void powerSleepEnterDeep() {
  Serial.println("Entering DEEP sleep (DISARMED) — wake on IMU motion");
  Serial.flush();

  prepareImuGpioWake();
  shutdownWifiForSleep();

  // In deep sleep the next instruction never runs (cus we turned off CPU and RAM)
  // state machine resets to DISARMED
  esp_deep_sleep_start();
}

// unfortunately the wifi access point does not work while in light sleep mode
void powerSleepEnterLight() {
  Serial.println("Entering LIGHT sleep (ARMED) — wake on IMU motion");
  Serial.flush();

  prepareImuGpioWake();

  // Light sleep RAM is retained, millis() continues, we resume here on wake
  esp_light_sleep_start();

  // woke up here
  logWakeCause();
  rtc_gpio_deinit((gpio_num_t)IMU_INT_PIN);
  pinMode(IMU_INT_PIN, INPUT);

  // Clear IMU INT so the next sleep doesn't instantly wake again
  motionSensorClearWakeInterrupt();

  Serial.println("Left LIGHT sleep");
}
