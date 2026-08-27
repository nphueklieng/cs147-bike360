#ifndef WIFI_COMM_H
#define WIFI_COMM_H

#include <Arduino.h>

// Phone sends one ASCII line:
//   ARM | DISARM | PING | STATUS
//
// Device sends one ASCII line back:
//   THEFT_DETECTED | ALARM_ACTIVE | CONNECTED | STATUS:... | PONG | ERR:UNKNOWN_CMD

bool wifiInit();          // start hotspot and TCP server (replaces bleInit)
void wifiUpdate();        // read commands and keep socket alive (replaces bleUpdate)
bool wifiIsConnected();   // does phone have an open TCP socket?
void wifiNotify(const char *message);  // push a line to the phone (replaces bleNotify)

#endif