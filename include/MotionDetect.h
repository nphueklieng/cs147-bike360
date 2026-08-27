#ifndef MOTION_DETECT_H
#define MOTION_DETECT_H

#include "MotionSensor.h"

// Clear the average buffer, streaks, and gravity estimate.
// called on boot, after ARM, and after an alarm finishes.
void motionDetectReset();

// Inputted one accelerometer sample.
// Returns true only when motion looks like theft
bool motionDetectUpdate(const AccelSample *sample);

// Most recent magnitude in g
float motionDetectLastMagnitude();

#endif
