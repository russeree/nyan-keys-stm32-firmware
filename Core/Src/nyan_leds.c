/**
 * @file nyan_leds.c
 * @brief Implementation of LED timing and control functionality.
 */
#include "nyan_leds.h"

/** 
 * @brief Frame rate at which the system status LED operates.
 */
const double SYSTEM_STATUS_LED_FPS = 255;

/** 
 * @brief Increment of radians for each frame in the system status LED animation.
 *
 * Calculated based on the 2π radians of a full circle divided by the number of frames per second.
 */
const double SYSTEM_STATUS_DEGREE_INCREMENT = (2 * 3.14)/SYSTEM_STATUS_LED_FPS;

/**
 * @brief Calculates the OCR value for system status LED based on the given angle in radians.
 *
 * This function takes an angle in radians and calculates the corresponding OCR value for the LED control.
 * It uses a cosine function to create a pulsing effect, with the formula [Sin^2(x) + Cos^2(x) = 1].
 *
 * @param radians The angle in radians for which to calculate the OCR value.
 * @return The calculated OCR value to control the LED brightness.
 */
unsigned char getSystemStatusOCRValue(double radians) {
    double t_on = pow(cosf(radians), 2);
    t_on *= 50;
    t_on += 5;

    return (unsigned char)floor(t_on);
}