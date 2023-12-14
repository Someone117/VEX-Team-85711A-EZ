/**
 * @file src/lemlib/util.cpp
 * @author Liam Teale
 * @brief File containing definitions for utility functions
 * @version 0.4.5
 * @date 2023-01-15
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "lemlib/util.hpp"

#include <math.h>

#include <algorithm>
#include <vector>

pros::Controller master(pros::E_CONTROLLER_MASTER);

/*omit
 * @brief Slew rate limiter
 *
 * @param target target value
 * @param current current value
 * @param maxChange maximum change. No maximum if set to 0
 * @return float - the limited value
 */
float lemlib::slew(float target, float current, float maxChange) {
  float change = target - current;
  if (maxChange == 0) return target;
  if (change > maxChange)
    change = maxChange;
  else if (change < -maxChange)
    change = -maxChange;
  return current + change;
}

/*omit
 * @brief Convert radians to degrees
 *
 * @param rad radians
 * @return float degrees
 */
float lemlib::radToDeg(float rad) { return rad * 180 / M_PI; }

/*omit
 * @brief Convert degrees to radians
 *
 * @param deg degrees
 * @return float radians
 */
float lemlib::degToRad(float deg) { return deg * M_PI / 180; }

/*omit
 * @brief Calculate the error between 2 angles. Useful when calculating the error between 2 headings
 *
 * @param angle1
 * @param angle2
 * @param radians true if angle is in radians, false if not. False by default
 * @return float wrapped angle
 */
float lemlib::angleError(float angle1, float angle2, bool radians) {
  float max = radians ? 2 * M_PI : 360;
  float half = radians ? M_PI : 180;
  angle1 = fmod(angle1, max);
  angle2 = fmod(angle2, max);
  float error = angle1 - angle2;
  if (error > half)
    error -= max;
  else if (error < -half)
    error += max;
  return error;
}

/*omit
 * @brief Return the sign of a number
 *
 * @param x the number to get the sign of
 * @return float - -1 if negative, 1 if positive
 */
float lemlib::sgn(float x) {
  if (x < 0)
    return -1;
  else
    return 1;
}

/*omit
 * @brief Return the average of a vector of numbers
 *
 * @param values
 * @return float
 */
float lemlib::avg(std::vector<float> values) {
  float sum = 0;
  for (float value : values) {
    sum += value;
  }
  return sum / values.size();
}

/*omit
 * @brief Return the average of a vector of numbers
 *
 * @param values
 * @return double
 */
double lemlib::avg(std::vector<double> values) {
  double sum = 0;
  for (double value : values) {
    sum += value;
  }
  return sum / values.size();
}

std::string lemlib::get_last_word(std::string text) {
  std::string word = "";
  for (int i = text.length() - 1; i >= 0; i--) {
    if (text[i] != ' ') {
      word += text[i];
    } else {
      std::reverse(word.begin(), word.end());
      return word;
    }
  }
  std::reverse(word.begin(), word.end());
  return word;
}
std::string lemlib::get_rest_of_the_word(std::string text, int position) {
  std::string word = "";
  for (int i = position; i < text.length(); i++) {
    if (text[i] != ' ' && text[i] != '\n') {
      word += text[i];
    } else {
      return word;
    }
  }
  return word;
}
// All iance\n\nWE WIN THESE!!!!!
void lemlib::print_to_screen(std::string text, int line) {
  int CurrAutoLine = line;
  std::vector<std::string> texts = {};
  std::string temp = "";

  for (int i = 0; i < text.length(); i++) {
    if (text[i] != '\n' && temp.length() + 1 > 32) {
      auto last_word = get_last_word(temp);
      if (last_word == temp) {
        texts.push_back(temp);
        temp = text[i];
      } else {
        int size = last_word.length();

        auto rest_of_word = get_rest_of_the_word(text, i);
        temp.erase(temp.length() - size, size);
        texts.push_back(temp);
        last_word += rest_of_word;
        i += rest_of_word.length();
        temp = last_word;
        if (i >= text.length() - 1) {
          texts.push_back(temp);
          break;
        }
      }
    }
    if (i >= text.length() - 1) {
      temp += text[i];
      texts.push_back(temp);
      temp = "";
      break;
    } else if (text[i] == '\n') {
      texts.push_back(temp);
      temp = "";
    } else {
      temp += text[i];
    }
  }
  for (auto i : texts) {
    if (CurrAutoLine > 7) {
      pros::lcd::clear();
      pros::lcd::set_text(line, "Out of Bounds. Print Line is too far down");
      return;
    }
    pros::lcd::clear_line(CurrAutoLine);
    pros::lcd::set_text(CurrAutoLine, i);
    CurrAutoLine++;
  }
}

double lemlib::curve_function(double v, double pow) {
  if (v > 0) {
    return (std::pow(std::abs(v), pow) / std::pow(100, pow)) * 127;
  } else {
    return -1 * (std::pow(std::abs(v), pow) / std::pow(100, pow)) * 127;
  }
}
