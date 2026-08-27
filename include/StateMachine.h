#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>

// device modes, we handle transitions in StateMachine.cpp.
enum DeviceState {
  STATE_DISARMED = 0,      // idle, motion ignored
  STATE_ARMED,             // watching for theft
  STATE_ALARM_TRIGGERED,   // brief handoff into sounding alarm
  STATE_ALARM_ACTIVE,      // buzzer on, waiting for DISARM or timeout
  STATE_RESETTING          // cleanup, then back to ARMED
};

void stateMachineInit();

// motionSuspected comes from MotionDetect
void stateMachineUpdate(bool motionSuspected);

// Called from Wifi command handling
void stateMachineArm();
void stateMachineDisarm();

DeviceState stateMachineGetState();
const char *stateMachineStateName();  // get label for STATUS

#endif
