#ifndef BLE_COMM_H
#define BLE_COMM_H

#include <Arduino.h>

// Phone writes ASCII commands to the CMD characteristic:
//   ARM | DISARM | PING | STATUS
//
// Device notifies on the NOTIFY characteristic:
//   THEFT_DETECTED | ALARM_ACTIVE | BATTERY_LOW
//   CONNECTED | DISCONNECTED | STATUS:... | PONG

bool bleInit();

// Call every loop: drains pending phone commands + restarts advertising.
void bleUpdate();

bool bleIsConnected();

// Send a short ASCII event to the phone (also printed on Serial).
void bleNotify(const char *message);

#endif
