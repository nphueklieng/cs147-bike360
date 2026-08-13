#include "MotionSensor.h"
#include "Config.h"
#include <Wire.h>
#include <math.h>

// MPU6050 registers we use to configure sensor/get data
static const uint8_t REG_SMPLRT_DIV   = 0x19;   // divide sample rate (controls how fast data updates)
static const uint8_t REG_CONFIG       = 0x1A;   // general config
static const uint8_t REG_ACCEL_CONFIG = 0x1C;   // set acc sensitivity range
static const uint8_t REG_PWR_MGMT_1   = 0x6B;   // manage power (wake sensor up from sleep)
static const uint8_t REG_ACCEL_XOUT_H = 0x3B;   // accelerometer data (x-axis high byte)
static const uint8_t REG_WHO_AM_I     = 0x75;   // reading this should return sensor's ID


// default full-scale after reset: accel +/-2g
static const float ACCEL_SCALE = 16384.0f;  // LSB per g


// I2C functions handle low level comm with the sensor

// Writes one byte to spec register on the sensor
static bool writeReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(reg);
  Wire.write(val);
  return Wire.endTransmission() == 0;
}

// Read len bytes starting at register reg into buffer buf
static bool readRegs(uint8_t reg, uint8_t *buf, uint8_t len) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  // request len bytes from the sensor
  uint8_t got = Wire.requestFrom((uint8_t)MPU6050_ADDR, len);
  if (got != len) {
    return false;
  }

  for (uint8_t i = 0; i < len; i++) {
    buf[i] = Wire.read();
  }
  return true;
}

// MPU6050 sends signed 16-bit big-endian values
static int16_t be16(const uint8_t *p) {
  return (int16_t)((p[0] << 8) | p[1]);
}


// initialize sensor: wake up sensor, check if its alive, and configures settings
bool motionSensorInit() {
  Serial.println("Initializing MPU6050...");

  // start I2C bus
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  delay(50);  // allow sensor power rail time to power up

  uint8_t who = 0;
  // check if device is responding
  if (!readRegs(REG_WHO_AM_I, &who, 1)) {
    Serial.println("MPU6050: I2C read failed (check wiring / address)");
    return false;
  }

  Serial.print("MPU6050 WHO_AM_I=0x");
  Serial.println(who, HEX);

  // normally should return 0x68
  if (who != 0x68 && who != 0x70 && who != 0x98) {
    Serial.println("MPU6050: unexpected WHO_AM_I (continuing anyway)");
  }

  // default, MPU6050 starts in sleep mode to save power. we clear that bit to wake it up
  if (!writeReg(REG_PWR_MGMT_1, 0x00)) {
    Serial.println("MPU6050: wake failed");
    return false;
  }
  delay(50);

  // Explicit ranges so we know the scale factors above are valid
  writeReg(REG_ACCEL_CONFIG, 0x00);  // +/- 2g
  writeReg(REG_CONFIG, 0x03);        // DLPF ~44 Hz (cuts a bit of noise)
  writeReg(REG_SMPLRT_DIV, 0x09);    // sample rate divider (with DLPF on)

  Serial.println("MPU6050 ready");
  return true;
}

bool motionSensorReadAccel(AccelSample *out) {
  // data reading accelerometer
  if (!out) {
    return false;
  }

  uint8_t raw[6];
  if (!readRegs(REG_ACCEL_XOUT_H, raw, 6)) {
    return false;
  }

  out->ax = be16(&raw[0]) / ACCEL_SCALE;
  out->ay = be16(&raw[2]) / ACCEL_SCALE;
  out->az = be16(&raw[4]) / ACCEL_SCALE;
  return true;
}


float accelMagnitude(const AccelSample *a) {
  // calculates overall acceleration vector
  if (!a) {
    return 0.0f;
  }
  return sqrtf(a->ax * a->ax + a->ay * a->ay + a->az * a->az);
}
