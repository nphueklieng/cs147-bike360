#ifndef CONFIG_H
#define CONFIG_H

// PINS
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
#define BUZZER_PIN 18

// 1 = HIGH turns buzzer on
#define BUZZER_ACTIVE_HIGH 1

// LSM6DSO
// 0x6B when SA0/SDO is HIGH 
#define LSM6DSO_ADDR 0x6B

// TIMING
#define SAMPLE_INTERVAL_MS 50       // how often we read the IMU
#define ALARM_DURATION_MS 30000     // stop alarm after this duration
#define ALARM_PULSE_ON_MS 200       // buzzer on-time per pulse
#define ALARM_PULSE_OFF_MS 150      // buzzer off-time per pulse
#define ARM_SETTLE_MS 2000          // ignore initial motion right after ARM
#define STATUS_LOG_INTERVAL_MS 1000 // Serial magnitude print rate
#define BLE_ADV_RESTART_MS 100      // wait before re-advertising

// MOTION DETECTION
// values are in g, after adjusting for gravity
#define MOTION_AVG_WINDOW 8      // samples in the moving average
#define MOTION_THRESHOLD_G 0.35f // sustained movement
#define IMPACT_THRESHOLD_G 1.20f // hard, rapid motions
#define MOTION_CONSEC_SAMPLES 6  // avg hits needed to trigger
#define IMPACT_CONSEC_SAMPLES 2  // impact hits needed to trigger

// WI-FI (we replaced BLE with Wi-Fi)
#define WIFI_SSID "Bike360"
#define WIFI_PASSWORD "bike360password"
#define WIFI_AP_CHANNEL 1   // there are 13 channels, Channel 1 is the default
#define WIFI_MAX_CLIENTS 4   // there's only 1 owner
#define WIFI_CMD_PORT 3333   // TCP port the phone connects to when it joins wifi

#define SERIAL_BAUD 115200

#endif
