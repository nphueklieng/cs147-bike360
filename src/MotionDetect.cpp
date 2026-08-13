#include "MotionDetect.h"
#include "Config.h"
#include <math.h>

// circular buffer of most recent magnitudes so we can calculate average
static float magBuf[MOTION_AVG_WINDOW];
static uint8_t magIndex = 0;
static uint8_t magCount = 0;

// how many recent samples were above the threshold (so they r suspicious)
static uint8_t motionStreak = 0;
static uint8_t impactStreak = 0;
static float lastDynMag = 0.0f;

// a lower threshold so we aren't flagging false alarms as suspicious
static float gEstX = 0.0f;
static float gEstY = 0.0f;
static float gEstZ = 1.0f;
static bool gReady = false;

static float movingAverage() {
  if (magCount == 0) {
    return 0.0f;
  }

  float sum = 0.0f;
  for (uint8_t i = 0; i < magCount; i++) {
    sum += magBuf[i];
  }
  return sum / (float)magCount;
}

void motionDetectReset() {
  magIndex = 0;
  magCount = 0;
  motionStreak = 0;
  impactStreak = 0;
  lastDynMag = 0.0f;
  gEstX = 0.0f;
  gEstY = 0.0f;
  gEstZ = 1.0f;
  gReady = false;

  for (uint8_t i = 0; i < MOTION_AVG_WINDOW; i++) {
    magBuf[i] = 0.0f;
  }
}

bool motionDetectUpdate(const AccelSample *sample) {
  if (!sample) {
    return false;
  }

  // we are using slow exponential moving average (EMA) to track gravity
  // it helps isolate fast/sudden movements by subtracting est. gravity from the current motion sample
  if (!gReady) {
    gEstX = sample->ax;
    gEstY = sample->ay;
    gEstZ = sample->az;
    gReady = true;
  } else {
    const float alpha = 0.02f;  // small = slower gravity tracking
    gEstX = gEstX * (1.0f - alpha) + sample->ax * alpha;
    gEstY = gEstY * (1.0f - alpha) + sample->ay * alpha;
    gEstZ = gEstZ * (1.0f - alpha) + sample->az * alpha;
  }

  float dx = sample->ax - gEstX;
  float dy = sample->ay - gEstY;
  float dz = sample->az - gEstZ;
  float dyn = sqrtf(dx * dx + dy * dy + dz * dz);
  lastDynMag = dyn;

  // add new info to the buffer
  magBuf[magIndex] = dyn;
  magIndex = (magIndex + 1) % MOTION_AVG_WINDOW;
  if (magCount < MOTION_AVG_WINDOW) {
    magCount++;
  }

  // the average is only meaningful when the buffer window is full
  if (magCount < MOTION_AVG_WINDOW) {
    return false;
  }

  float avg = movingAverage();

  // hard impact (check against threshold)
  if (dyn >= IMPACT_THRESHOLD_G) {
    impactStreak++;
  } else {
    impactStreak = 0;
  }

  // long shake / lift (uses smoothed average)
  if (avg >= MOTION_THRESHOLD_G) {
    motionStreak++;
  } else if (motionStreak > 0) {
    // slowly decrement because sensor readings can be very noisy
    motionStreak--;
  }

  if (impactStreak >= IMPACT_CONSEC_SAMPLES) {
    Serial.println("MotionDetect: impact pattern");
    impactStreak = 0;
    motionStreak = 0;
    return true;
  }

  if (motionStreak >= MOTION_CONSEC_SAMPLES) {
    Serial.println("MotionDetect: sustained motion");
    impactStreak = 0;
    motionStreak = 0;
    return true;
  }

  return false;
}

float motionDetectLastMagnitude() {
  return lastDynMag;
}
