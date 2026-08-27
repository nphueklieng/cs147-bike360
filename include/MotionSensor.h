#ifndef MOTION_SENSOR_H
#define MOTION_SENSOR_H

#include <Arduino.h>

// 1 accelerometer reading in g 
struct AccelSample {
  float ax;
  float ay;
  float az;
};

// One gyro reading in degrees/second
struct GyroSample {
  float gx;
  float gy;
  float gz;
};

// Wake MPU6050 over I2C. true = ready to read
bool motionSensorInit();

// update *out to the latest accel/gyro sample. false = bus/sensor error
bool motionSensorReadAccel(AccelSample *out);
bool motionSensorReadGyro(GyroSample *out);
bool motionSensorEnableWakeInterrupt();  // wake interrupt
void motionSensorClearWakeInterrupt();   // clear the INT so we can sleep again

// |a| = sqrt(ax^2 + ay^2 + az^2). Includes gravity when at rest
float accelMagnitude(const AccelSample *a);

#endif
