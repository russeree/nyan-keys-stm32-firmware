/**
 * Portland.HODL
 * LED timing and control functionality.
 */

#include "nyan_leds.h"

#include <math.h>

const double SYSTEM_STATUS_LED_FPS = 255;
const double SYSTEM_STATUS_DEGREE_INCREMENT = (2 * 3.14)/SYSTEM_STATUS_LED_FPS;

unsigned char getSystemStatusOCRValue (double radians) {
    double t_on = pow(cosf(radians),2);
    t_on *= 50;
    t_on += 5;

    return (unsigned char)floor(t_on);
}