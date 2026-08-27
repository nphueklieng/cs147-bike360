#include "Alarm.h"
#include "Config.h"

static bool active = false;     // is the alarm running
static bool pinOn = false;      
static uint32_t startedAt = 0;  // when startAlarm() was called
static uint32_t pulseAt = 0;    // last time we turned the pin on or off

static void buzzerWrite(bool on) {
  if (on) {
    tone(BUZZER_PIN, 1000);
  } else {
    noTone(BUZZER_PIN);
  }
}

void alarmInit() {
  pinMode(BUZZER_PIN, OUTPUT);
  buzzerWrite(false);
  active = false;
  pinOn = false;
}

void startAlarm() {
  active = true;
  startedAt = millis();
  pulseAt = startedAt;
  pinOn = true;
  buzzerWrite(true);
  Serial.println("Alarm activated");
}

void stopAlarm() {
  // so that we aren't spamming alarm stop command
  if (!active && !pinOn) {
    buzzerWrite(false);
    return;
  }

  active = false;
  pinOn = false;
  buzzerWrite(false);
  Serial.println("Alarm stopped");
}

void updateAlarm() {
  if (!active) {
    return;
  }

  uint32_t now = millis();
  uint32_t period = pinOn ? ALARM_PULSE_ON_MS : ALARM_PULSE_OFF_MS;

  // flip alarm on/off without blocking everything else
  if ((now - pulseAt) >= period) {
    pinOn = !pinOn;
    buzzerWrite(pinOn);
    pulseAt = now;
  }
}

bool alarmIsActive() {
  return active;
}

bool alarmTimedOut() {
  if (!active) {
    return false;
  }
  return (millis() - startedAt) >= ALARM_DURATION_MS;
}
