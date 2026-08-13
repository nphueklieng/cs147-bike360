#include "MotionSensor.h"
#include "Config.h"
#include <Wire.h>
#include <math.h>
#include <SparkFunLSM6DSO.h>

static LSM6DSO imu;
static bool imuReady = false;

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
