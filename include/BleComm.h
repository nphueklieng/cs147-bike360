#ifndef BLE_COMM_H
#define BLE_COMM_H

#include <Arduino.h>

// Phone writes ASCII commands:
//   ARM | DISARM | PING | STATUS
//
// Device notifies:
//   THEFT_DETECTED | ALARM_ACTIVE | BATTERY_LOW
//   CONNECTED | DISCONNECTED | STATUS:... | PONG

bool bleInit();

// call every loop: get the new phone commands + restarts advertising.
void bleUpdate();

bool bleIsConnected();

// send a short message to the phone
void bleNotify(const char *message);

#endif
