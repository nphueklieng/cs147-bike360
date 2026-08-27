#include "MotionSensor.h"
#include "Config.h"
#include "esp_sleep.h"
#include <Wire.h>
#include <math.h>
#include <SparkFunLSM6DSO.h>

static LSM6DSO imu;
static bool imuReady = false;

bool motionSensorEnableWakeInterrupt() {
  if (!imuReady) return false;

  // keep the accelerometer running so the wake engine has samples
  imu.setAccelRange(2);
  imu.setAccelDataRate(104);  // this is enough sensitivity and low enough to save power
  imu.setGyroDataRate(0); // gyro is useless when asleep
  pinMode(IMU_INT_PIN, INPUT);

  imu.writeRegister(INT1_CTRL, 0x00); // clear register

  // we didnt use routeHardInterOne() SparkFun helper cus apparently it has a bug that forces SINGLE_TAP routing.
  uint8_t tapCfg2 = 0;
  if (imu.readRegister(&tapCfg2, TAP_CFG2) != IMU_SUCCESS) return false;
  tapCfg2 |= INTERRUPTS_ENABLED;  // 0x80
  if (imu.writeRegister(TAP_CFG2, tapCfg2) != IMU_SUCCESS) return false;
  
  // Threshold: only lower 6 bits are wake threshold
  uint8_t ths = (uint8_t)(IMU_INT_WAKE_THRESHOLD & WK_THS_MASK);
  if (imu.writeRegister(WAKE_UP_THS, ths) != IMU_SUCCESS) return false;
  
  // wake as soon as it exceeds threshold.
  if (imu.writeRegister(WAKE_UP_DUR, 0x00) != IMU_SUCCESS) return false;
  
  // Route wake up event to INT1 pin
  if (imu.writeRegister(MD1_CFG, INT1_WU_ENABLED) != IMU_SUCCESS) return false;
  Serial.print("IMU wake interrupt armed, WK_THS=");
  Serial.println(ths);
  return true;
}

void motionSensorClearWakeInterrupt() {
  if (!imuReady) return;
  uint8_t dummy = 0;
  // clears the interrupt flags
  imu.readRegister(&dummy, WAKE_UP_SRC);
  imu.readRegister(&dummy, ALL_INT_SRC);
  (void)dummy;
}

// Initialize sensor: start I2C, connect to LSM6DSO, configure sampling ranges/rates
bool motionSensorInit()
{
  Serial.println("Initializing LSM6DSO...");

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  delay(50);

  if (!imu.begin())
  {
    Serial.println("Could not connect to LSM6DSO. Check I2C wiring.");
    return false;
  }

  if (!imu.initialize(BASIC_SETTINGS))
  {
    Serial.println("IMU found but initialize(BASIC_SETTINGS) failed.");
  } else {
    Serial.println("LSM6DSO ready.");
  }

  // +/-2g range gives the best resolution for theft detection
  imu.setAccelRange(2);
  imu.setAccelDataRate(104);
  imu.setGyroRange(250);
  imu.setGyroDataRate(104);

  Serial.println("LSM6DSO ready");
  imuReady = true;
  return true;
}

bool motionSensorReadAccel(AccelSample *out)
{
  if (!out || !imuReady)
  {
    return false;
  }

  // read all 3 axes, already converted to g
  out->ax = imu.readFloatAccelX();
  out->ay = imu.readFloatAccelY();
  out->az = imu.readFloatAccelZ();
  return true;
}

bool motionSensorReadGyro(GyroSample *out)
{
  if (!out || !imuReady)
  {
    return false;
  }

  out->gx = imu.readFloatGyroX();
  out->gy = imu.readFloatGyroY();
  out->gz = imu.readFloatGyroZ();
  return true;
}

float accelMagnitude(const AccelSample *a)
{
  if (!a)
  {
    return 0.0f;
  }
  return sqrtf(a->ax * a->ax + a->ay * a->ay + a->az * a->az);
}
