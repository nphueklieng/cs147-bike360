#ifndef CONFIG_H
#define CONFIG_H

// ---- pins ----
// ESP32 default I2C pins. Remap if your PCB uses different GPIOs.
#define I2C_SDA_PIN        21
#define I2C_SCL_PIN        22
#define BUZZER_PIN         18

// 1 = HIGH turns buzzer on (typical active buzzer module)
// 0 = LOW turns buzzer on (active-low)
#define BUZZER_ACTIVE_HIGH 1

// ---- MPU6050 ----
// 0x68 when AD0 is tied to GND; 0x69 if AD0 is HIGH
#define MPU6050_ADDR       0x69

// ---- timing ----
#define SAMPLE_INTERVAL_MS     50      // how often we read the IMU
#define ALARM_DURATION_MS      30000   // auto-stop after this long
#define ALARM_PULSE_ON_MS      200     // buzzer on-time per pulse
#define ALARM_PULSE_OFF_MS     150     // buzzer off-time per pulse
#define ARM_SETTLE_MS          2000    // ignore initial motion right after ARM
#define STATUS_LOG_INTERVAL_MS 1000    // Serial magnitude print rate
#define BLE_ADV_RESTART_MS     100     // wait before re-advertising

// ---- motion detection ----
// Values are in g, after roughly removing gravity.
// Start conservative; lower MOTION_THRESHOLD_G if it misses real thefts.
#define MOTION_AVG_WINDOW      8       // samples in the moving average
#define MOTION_THRESHOLD_G     0.35f   // sustained shake / lift
#define IMPACT_THRESHOLD_G     1.20f   // hard hit
#define MOTION_CONSEC_SAMPLES  6       // avg hits needed to trigger
#define IMPACT_CONSEC_SAMPLES  2       // impact hits needed to trigger

// ---- BLE ----
#define BLE_DEVICE_NAME    "Bike360"

// Custom UUIDs. Must match the phone app.
#define BLE_SERVICE_UUID           "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define BLE_CMD_CHAR_UUID          "6e400002-b5a3-f393-e0a9-e50e24dcca9e"   // phone writes here
#define BLE_NOTIFY_CHAR_UUID       "6e400003-b5a3-f393-e0a9-e50e24dcca9e"   // we notify here

#define SERIAL_BAUD 115200

#endif
