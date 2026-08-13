#include "BleComm.h"
#include "Config.h"
#include "StateMachine.h"

#include <string.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

static BLEServer *server = nullptr;
static BLECharacteristic *cmdChar = nullptr;
static BLECharacteristic *notifyChar = nullptr;

static bool deviceConnected = false;
static bool wasConnected = false;

// advertise again after disconnect without blocking other stuff
static bool needAdvRestart = false;
static uint32_t advRestartAt = 0;

// store the command and just handle it in bleUpdate() in loop().
static volatile bool cmdPending = false;
static char pendingCmd[33];

static void trimInPlace(char *s) {
  size_t n = strlen(s);
  while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r' || s[n - 1] == ' ')) {
    s[--n] = '\0';
  }
}

static void toUpperInPlace(char *s) {
  for (char *p = s; *p; p++) {
    if (*p >= 'a' && *p <= 'z') {
      *p = (char)(*p - 'a' + 'A');
    }
  }
}

static void handleCommand(const char *cmd) {
  Serial.print("BLE cmd: ");
  Serial.println(cmd);

  if (strcmp(cmd, "ARM") == 0) {
    stateMachineArm();
  } else if (strcmp(cmd, "DISARM") == 0) {
    stateMachineDisarm();
  } else if (strcmp(cmd, "PING") == 0) {
    bleNotify("PONG");  // tells us that BLE is up and notifications are working
  } else if (strcmp(cmd, "STATUS") == 0) {
    char buf[48];
    snprintf(buf, sizeof(buf), "STATUS:%s", stateMachineStateName());
    bleNotify(buf);
  } else {
    Serial.println("BLE: unknown command");
    bleNotify("ERR:UNKNOWN_CMD");
  }
}

// Arduino BLE requires this small callback class
class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *s) override {
    (void)s;
    deviceConnected = true;
    Serial.println("BLE Connected");
  }

  void onDisconnect(BLEServer *s) override {
    (void)s;
    deviceConnected = false;
    // Schedule advertising restart; don't delay() inside the callback
    needAdvRestart = true;
    advRestartAt = millis() + BLE_ADV_RESTART_MS;
    Serial.println("BLE Disconnected");
  }
};

class CmdCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *c) override {
    std::string value = c->getValue();
    if (value.length() == 0 || value.length() > 32) {
      return;
    }

    // Drop if a command is already waiting (phone can retry)
    if (cmdPending) {
      return;
    }

    size_t n = value.length();
    memcpy(pendingCmd, value.data(), n);
    pendingCmd[n] = '\0';
    trimInPlace(pendingCmd);
    toUpperInPlace(pendingCmd);

    cmdPending = true;  // main loop will pick this up
  }
};

bool bleInit() {
  Serial.println("Starting BLE...");
  BLEDevice::init(BLE_DEVICE_NAME);

  server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  BLEService *svc = server->createService(BLE_SERVICE_UUID);

  // Phone sends ESP32 commands
  cmdChar = svc->createCharacteristic(
      BLE_CMD_CHAR_UUID,
      BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
  cmdChar->setCallbacks(new CmdCallbacks());

  // ESP32 sends phone events (enable notifications on the phone side)
  notifyChar = svc->createCharacteristic(
      BLE_NOTIFY_CHAR_UUID,
      BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ);
  notifyChar->addDescriptor(new BLE2902());

  svc->start();

  BLEAdvertising *adv = BLEDevice::getAdvertising();
  adv->addServiceUUID(BLE_SERVICE_UUID);
  adv->setScanResponse(true);
  // preferred connection intervals
  adv->setMinPreferred(0x06);
  adv->setMaxPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.print("BLE advertising as ");
  Serial.println(BLE_DEVICE_NAME);
  return true;
}

void bleUpdate() {
  // run any command the phone wrote
  if (cmdPending) {
    char local[33];
    memcpy(local, pendingCmd, sizeof(local));
    cmdPending = false;
    handleCommand(local);
  }

  // after a disconnect u can start advertising again after a short wait
  if (needAdvRestart && (int32_t)(millis() - advRestartAt) >= 0) {
    needAdvRestart = false;
    if (server != nullptr) {
      server->startAdvertising();
      Serial.println("BLE: advertising restarted");
    }
  }

  // if just connected we tell the app
  if (deviceConnected && !wasConnected) {
    bleNotify("CONNECTED");
  }

  // if connection just dropped we show it on serial
  if (!deviceConnected && wasConnected) {
    Serial.println("BLE notify: DISCONNECTED");
  }

  wasConnected = deviceConnected;
}

bool bleIsConnected() {
  return deviceConnected;
}

void bleNotify(const char *message) {
  if (!notifyChar || !message) {
    return;
  }

  notifyChar->setValue(message);
  if (deviceConnected) {
    notifyChar->notify();
  }

  // print to serial so you can debug without phone connected
  Serial.print("BLE notify: ");
  Serial.println(message);
}
