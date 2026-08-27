#include "WifiComm.h"
#include "Config.h"
#include "StateMachine.h"

#include <WiFi.h>      // ESP32 Wi-Fi built in
#include <string.h>

// TCP server waits for the phone to connect
static WiFiServer server(WIFI_CMD_PORT);

static WiFiClient client;

// Same as BLE: store the command and just handle it in wifiUpdate() in loop().
static volatile bool cmdPending = false;
static char pendingCmd[33];

// Strip newlines and spaces
static void trimInPlace(char *s) {
  size_t n = strlen(s);
  while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r' || s[n - 1] == ' ')) {
    s[--n] = '\0';
  }
}

// Make commands case-insensitive
static void toUpperInPlace(char *s) {
  for (char *p = s; *p; p++) {
    if (*p >= 'a' && *p <= 'z') {
      *p = (char)(*p - 'a' + 'A');
    }
  }
}

static void handleCommand(const char *cmd) {
  Serial.print("WiFi cmd: ");
  Serial.println(cmd);

  if (strcmp(cmd, "ARM") == 0) {
    stateMachineArm();
  } else if (strcmp(cmd, "DISARM") == 0) {
    stateMachineDisarm();
  } else if (strcmp(cmd, "PING") == 0) {
    wifiNotify("PONG");           // prove the socket can send back
  } else if (strcmp(cmd, "STATUS") == 0) {
    char buf[48];
    snprintf(buf, sizeof(buf), "STATUS:%s", stateMachineStateName());
    wifiNotify(buf);
  } else {
    Serial.println("WiFi: unknown command");
    wifiNotify("ERR:UNKNOWN_CMD");
  }
}

bool wifiInit() {
  Serial.println("Starting WiFi access point...");

  // ESP32 is the router, and phone joins "Bike360"
  bool ok = WiFi.softAP(WIFI_SSID, WIFI_PASSWORD, WIFI_AP_CHANNEL, 0, WIFI_MAX_CLIENTS);
  if (!ok) {
    Serial.println("WiFi AP failed");
    return false;
  }

  // start listening for TCP connections on WIFI_CMD_PORT.
  server.begin();

  Serial.print("WiFi AP IP: ");
  Serial.println(WiFi.softAPIP());   // phone connects here
  Serial.print("TCP port: ");
  Serial.println(WIFI_CMD_PORT);
  return true;
}

void wifiUpdate() {
  // If nobody is connected, accept a new phone
  if (!client || !client.connected()) {
    WiFiClient incoming = server.available();  // returns empty if no one is waiting so its nonblocking
    if (incoming) {
      client = incoming;
      client.setTimeout(10);                 
      Serial.println("WiFi Connected");
      wifiNotify("CONNECTED");
    }
  }

  // Read one line if the phone sent data. BLE used onWrite(), but here poll the socket.
  if (client && client.connected() && client.available() && !cmdPending) {
    String line = client.readStringUntil('\n');
    line.trim();
    if (line.length() > 0 && line.length() <= 32) {
      strncpy(pendingCmd, line.c_str(), sizeof(pendingCmd) - 1);
      pendingCmd[sizeof(pendingCmd) - 1] = '\0';
      toUpperInPlace(pendingCmd);
      cmdPending = true;
    }
  }

  // Handle the command on the main loop
  if (cmdPending) {
    char local[33];
    memcpy(local, pendingCmd, sizeof(local));
    cmdPending = false;
    handleCommand(local);
  }
}

bool wifiIsConnected() {
  return client && client.connected();
}

void wifiNotify(const char *message) {
  if (!message) {
    return;
  }

  // Push to the phone if the socket is open like in bleNotify()
  if (client && client.connected()) {
    client.print(message);
    client.print("\n"); 
  }

  Serial.print("WiFi notify: ");
  Serial.println(message);
}


void wifiShutdown() {
  // Drop the TCP client first so the phone gets clean disconnect
  if (client) {
    client.stop();
  }
  server.end();

  // take down the SoftAP, then power the Wi-Fi modem off
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.println("WiFi shut down for sleep");
}