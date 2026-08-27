#include <Arduino.h>

#include "Config.h"
#include "MotionSensor.h"
#include "MotionDetect.h"
#include "Alarm.h"
#include "WifiComm.h"
#include "StateMachine.h"
#include "PowerSleep.h"

/*
 Bike360

 Main loop:
  1) service BLE (commands + advertising)
  2) pulse the buzzer if alarming
  3) each sample: read accelerometer -> check threshold -> update state
 
 */

static uint32_t lastSampleMs = 0;
static uint32_t lastStatusLogMs = 0;
static bool sensorOk = false;

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(200);
  Serial.println();
  Serial.println("=== Bike360 boot ===");

  // Log wake cause and start DISARMED awake window before Wi-Fi/state.
  powerSleepInit();

  // Hardware / logic init outputs first, then sensing, then radio
  alarmInit();
  motionDetectReset();
  stateMachineInit();

  sensorOk = motionSensorInit();
  if (!sensorOk) {
    Serial.println("Sensor init failed: motion checks disabled");
  }

  // If we woke because the bike moved while DISARMED, refresh the window
  if (powerSleepWokeFromMotion()) {
    powerSleepRefreshDisarmAwakeWindow();
    Serial.println("Motion wake while (will be) DISARMED — Wi-Fi up for ARM");
  }

  wifiInit();

  lastSampleMs = millis();
  lastStatusLogMs = millis();
  Serial.println("Init complete");
}

void loop() {
  uint32_t now = millis();

  wifiUpdate();

  // Check if the alarm should be active. pulse buzzer if active
  updateAlarm();

  // read accelerometer if enough time has passed
  if ((now - lastSampleMs) < SAMPLE_INTERVAL_MS) {
    return;
  }
  lastSampleMs = now;

  bool motion = false;

  if (sensorOk) {
    AccelSample accel;
    if (motionSensorReadAccel(&accel)) {

      motion = motionDetectUpdate(&accel);

      // just for printing the accelerometer status periodically
      // if ((now - lastStatusLogMs) >= STATUS_LOG_INTERVAL_MS) {
      //   lastStatusLogMs = now;
      //   Serial.print("Motion magnitude: ");
      //   Serial.print(motionDetectLastMagnitude(), 3);
      //   Serial.print(" g | state=");
      //   Serial.println(stateMachineStateName());
      // }
    } else {
      Serial.println("MPU6050 read failed");
    }
  }

  // our state machine decides state (armed/alarm/reset) based on motion & timers
  stateMachineUpdate(motion);
}
