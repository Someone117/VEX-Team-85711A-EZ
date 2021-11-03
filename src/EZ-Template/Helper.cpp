#include "EZ-Template/Helper.hpp"
#include "main.h"
namespace ez{
  namespace util{
    bool IS_SD_CARD = pros::usd::is_installed();
bool is_reversed(int input) {
  if (input < 0)
    return true;
  return false;
}

int sgn (int input) {
  if (input > 0)
    return 1;
  else if (input < 0)
    return -1;
  return 0;
}

double clip_num(double input, double max, double min) {
  if (input > max)
    return max;
  else if (input < min)
    return min;
  return input;
}
} // util namespace
} // ez namespace
