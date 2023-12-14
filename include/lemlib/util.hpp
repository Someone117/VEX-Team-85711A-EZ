/**
 * @file include/lemlib/util.hpp
 * @author LemLib Team
 * @brief Utility functions declarations
 * @version 0.4.5
 * @date 2023-01-15
 *
 * @copyright Copyright (c) 2023
 *
 */

#pragma once

#include <string>
#include <vector>

#include "api.h"

extern pros::Controller master;

namespace lemlib {
/**
 * @brief Slew rate limiter
 *
 * @param target target value
 * @param current current value
 * @param maxChange maximum change. No maximum if set to 0

 * @return float - the limited value
 */
float slew(float target, float current, float maxChange);

/**
 * @brief Convert radians to degrees
 *
 * @param rad radians
 * @return float degrees
 */
float radToDeg(float rad);

/**
 * @brief Convert degrees to radians
 *
 * @param deg degrees
 * @return float radians
 */
float degToRad(float deg);

/**
 * @brief Calculate the error between 2 angles. Useful when calculating the error between 2 headings
 *
 * @param angle1
 * @param angle2
 * @param radians true if angle is in radians, false if not. False by default
 * @return float wrapped angle
 */
float angleError(float angle1, float angle2, bool radians = false);

/**
 * @brief Return the sign of a number
 *
 * @param x the number to get the sign of
 * @return float - -1 if negative, 1 if positive
 */
float sgn(float x);

/**
 * @brief Return the average of a vector of numbers
 *
 * @param values
 * @return float
 */
float avg(std::vector<float> values);

/**
 * @brief Return the average of a vector of numbers
 *
 * @param values
 * @return double
 */
double avg(std::vector<double> values);
std::string get_last_word(std::string text);
std::string get_rest_of_the_word(std::string text, int position);
/**
 * Prints to the brain screen in one string.  Splits input between lines with
 * '\n' or when text longer then 32 characters.
 *
 * @param text
 *        Input string.  Use '\n' for a new line
 * @param line
 *        Starting line to print on, defaults to 0
 */
void print_to_screen(std::string text, int line = 0);

/**
 * Delay time for tasks
 */
const int DELAY_TIME = 10;

double curve_function(double v, double pow);
}  // namespace lemlib
