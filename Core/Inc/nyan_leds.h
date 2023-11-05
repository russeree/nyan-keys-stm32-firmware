#ifndef _NYAN_LEDS
#define _NYAN_LEDS

#define _USE_MATH_DEFINES
#include <math.h>

#define M_PI 3.14159265358979323846

extern const double SYSTEM_STATUS_LED_FPS;
extern const double SYSTEM_STATUS_DEGREE_INCREMENT;

/**
 * Main status LED - LED0 - PD0 
 * This led should have a continuous sin^2(x) + cos^2(x) and angle r rotates in 
 * perpetuity. Scale the result by 2*8-1
 */
unsigned char getSystemStatusOCRValue(double radians);


#endif