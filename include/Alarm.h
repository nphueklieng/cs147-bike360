#ifndef ALARM_H
#define ALARM_H

#include <Arduino.h>

void alarmInit();   // set buzzer pin, ensure off
void startAlarm();  // begin pulsing
void stopAlarm();   // force off (safe to call when already stopped)

// Drive the pulse pattern. Call often from loop(); uses millis(), no delay().
void updateAlarm();

bool alarmIsActive();

// true when ALARM_DURATION_MS has elapsed since startAlarm().
bool alarmTimedOut();

#endif
