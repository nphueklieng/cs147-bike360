#include "StateMachine.h"
#include "Config.h"
#include "Alarm.h"
#include "MotionDetect.h"
#include "WifiComm.h"

static DeviceState state = STATE_DISARMED;
static uint32_t stateEnteredAt = 0;
static uint32_t armedAt = 0;

// After ARM we ignore motion for a time period
// then, wipe detector history so we don't instantly trip the alarm initially
static bool settleCleared = false;


// transition states
static void enterState(DeviceState next) {
  if (state == next) {
    return;
  }

  // update to new state
  state = next;
  stateEnteredAt = millis();

  Serial.print("State -> ");
  Serial.println(stateMachineStateName());
}

void stateMachineInit() {
  // when device powers on, set the init baseline
  state = STATE_DISARMED;
  stateEnteredAt = millis();
  armedAt = 0;
  settleCleared = false;
  Serial.println("Device state: DISARMED");
}

void stateMachineArm() {
  // ignore duplicate ARM commands
  if (state == STATE_ARMED || state == STATE_ALARM_TRIGGERED ||
      state == STATE_ALARM_ACTIVE) {
    Serial.println("ARM ignored (already armed/alarming)");
    return;
  }

  // start looking out for sus motion
  motionDetectReset();
  stopAlarm();
  settleCleared = false;
  armedAt = millis();
  enterState(STATE_ARMED);
  Serial.println("Device armed");
  bleNotify("STATUS:ARMED");
}

void stateMachineDisarm() {
  // tells device to chill and turn off any active alarms
  stopAlarm();
  motionDetectReset();
  settleCleared = false;
  enterState(STATE_DISARMED);
  Serial.println("Device disarmed");
  bleNotify("STATUS:DISARMED");
}

void stateMachineUpdate(bool motionSuspected) {
  // is called constantly in main loop
  // evaluates currenft state, check time, and reacts to motion
  switch (state) {
    case STATE_DISARMED:
      // literally do nothing, ignore all motion. only able to leave this state if call stateMachineArm()
      // TODO: sleep
      break;

    case STATE_ARMED: {
      // Give the user time to clip the device on / walk away
      // TODO: we want the device to sleep to save power until a movement wakes it up
      if ((millis() - armedAt) < ARM_SETTLE_MS) {
        break;
      }

      // First sample after settle: clear any initial noise built up while attaching
      if (!settleCleared) {
        motionDetectReset();
        settleCleared = true;
        Serial.println("Arm settle done - monitoring");
        break;
      }

      // active monitoring
      if (motionSuspected) {
        Serial.println("Theft detected!");
        enterState(STATE_ALARM_TRIGGERED);
      }
      break;
    }

    case STATE_ALARM_TRIGGERED:
      // turns everything on, notifies phone, and immediately jumps to ALARM_ACTIVE
      startAlarm();
      bleNotify("THEFT_DETECTED");
      bleNotify("ALARM_ACTIVE");
      enterState(STATE_ALARM_ACTIVE);
      break;

    case STATE_ALARM_ACTIVE:
      // just waiting to see if alarm is timed out
      if (alarmTimedOut()) {
        Serial.println("Alarm timeout - resetting");
        enterState(STATE_RESETTING);
      }
      break;

    case STATE_RESETTING:
      // Stop buzzing but go back to armed mode
      stopAlarm();
      motionDetectReset();
      settleCleared = false;
      armedAt = millis();
      enterState(STATE_ARMED);
      bleNotify("STATUS:ARMED");
      break;
  }

  (void)stateEnteredAt;  // kept for future timeouts
}

DeviceState stateMachineGetState() {
  return state;
}

const char *stateMachineStateName() {
  switch (state) {
    case STATE_DISARMED:        return "DISARMED";
    case STATE_ARMED:           return "ARMED";
    case STATE_ALARM_TRIGGERED: return "ALARM_TRIGGERED";
    case STATE_ALARM_ACTIVE:    return "ALARM_ACTIVE";
    case STATE_RESETTING:       return "RESETTING";
    default:                    return "UNKNOWN";
  }
}
