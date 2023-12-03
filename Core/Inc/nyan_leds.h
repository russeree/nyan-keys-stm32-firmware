/**
 * @file nyan_leds.h
 * @brief Header file for LED control functionality.
 */

#ifndef _NYAN_LEDS
#define _NYAN_LEDS

#define _USE_MATH_DEFINES
#include <math.h>

#define M_PI 3.14159265358979323846

/**
 * @brief Frame rate for the system status LED.
 */
extern const double SYSTEM_STATUS_LED_FPS;

/**
 * @brief Radian increment for each frame in the system status LED animation.
 */
extern const double SYSTEM_STATUS_DEGREE_INCREMENT;

/**
 * @brief Calculates the OCR value for system status LED based on angle in radians.
 *
 * This function calculates the Output Compare Register (OCR) value for the system status LED
 * control. The brightness of the LED is determined by a combined sinusoidal and cosinusoidal
 * function over time.
 *
 * @param radians The angle in radians.
 * @return The calculated OCR value for LED brightness control.
 */
unsigned char getSystemStatusOCRValue(double radians);

#endif