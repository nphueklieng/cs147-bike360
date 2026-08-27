#ifndef ALARM_H
#define ALARM_H

#include <Arduino.h>

void alarmInit();   // set buzzer pin, make sure its off
void startAlarm();  // begin pulsing
void stopAlarm();   // force alarm to turn off 

// for the buzzer pattern
void updateAlarm();

bool alarmIsActive();

// check if time since startAlarm() is ALARM_DURATION_MS

#endif
