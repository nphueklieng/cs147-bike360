#ifndef POWER_SLEEP_H
#define POWER_SLEEP_H

#include <Arduino.h>

// power-on or IMU motion wake and starts the DISARMED
void powerSleepInit();

// Extend or restart the DISARMED "stay awake" window.
void powerSleepRefreshDisarmAwakeWindow();

// True when DISARM_AWAKE_MS has elapsed since the last refresh.
bool powerSleepDisarmAwakeExpired();

// Configure wake-up interrupt, shutdown Wi-Fi, then enter deep sleep.
void powerSleepEnterDeep();

// IMU/GPIO prep, shutdown Wi-Fi, enter light sleep.
// Restarts Wi-Fi softAP.
void powerSleepEnterLight();

bool powerSleepWokeFromMotion();

#endif