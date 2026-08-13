#ifndef MOTION_DETECT_H
#define MOTION_DETECT_H

#include "MotionSensor.h"

// Clear average buffer, streaks, and gravity estimate.
// Call on boot, after ARM, and after an alarm finishes.
void motionDetectReset();

// Feed one accel sample.
// Returns true only when motion looks like theft (not a single noisy spike).
bool motionDetectUpdate(const AccelSample *sample);

// Most recent dynamic magnitude in g (gravity removed). For Serial tuning.
float motionDetectLastMagnitude();

#endif
