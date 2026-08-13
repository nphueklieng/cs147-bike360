#ifndef MOTION_SENSOR_H
#define MOTION_SENSOR_H

#include <Arduino.h>

// One accelerometer reading in g (1g ~= gravity).
struct AccelSample {
  float ax;
  float ay;
  float az;
};

// One gyro reading in degrees/second (optional for now).
struct GyroSample {
  float gx;
  float gy;
  float gz;
};

// Wake MPU6050 over I2C (or enable simulator). true = ready to read.
bool motionSensorInit();

// Fill *out with the latest accel/gyro sample. false = bus/sensor error.
bool motionSensorReadAccel(AccelSample *out);
bool motionSensorReadGyro(GyroSample *out);

// |a| = sqrt(ax^2 + ay^2 + az^2). Includes gravity when at rest (~1.0).
float accelMagnitude(const AccelSample *a);

#endif
