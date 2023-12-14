/**
 * @file src/lemlib/chassis/chassis.cpp
 * @author LemLib Team
 * @brief definitions for the chassis class
 * @version 0.4.5
 * @date 2023-01-27
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "lemlib/chassis/chassis.hpp"

#include <math.h>

#include "..\..\..\include\constants.hpp"
#include "api.h"
#include "lemlib/chassis/odom.hpp"
#include "lemlib/pid.hpp"
#include "lemlib/util.hpp"
#include "pros/misc.hpp"
#include "pros/motors.hpp"
#include "trackingWheel.hpp"

/**
 * @brief Construct a new Chassis
 *
 * @param drivetrain drivetrain to be used for the chassis
 * @param lateralSettings settings for the lateral controller
 * @param angularSetting settings for the angular controller
 * @param sensors sensors to be used for odometry
 */
lemlib::Chassis::Chassis(Drivetrain_t drivetrain, ChassisController_t lateralSettings,
                         ChassisController_t angularSettings, OdomSensors_t sensors) {
  this->drivetrain = drivetrain;
  this->lateralSettings = lateralSettings;
  this->angularSettings = angularSettings;
  this->odomSensors = sensors;
}

/**
 * @brief Calibrate the chassis sensors
 *
 */
void lemlib::Chassis::calibrate() {
  // calibrate the imu if it exists
  if (odomSensors.imu != nullptr) {
    odomSensors.imu->reset();
    int iter = 0;
    while (true) {
      iter += 10;

      imu_loading_display(iter);

      if (iter >= 2000) {
        if (!(odomSensors.imu->get_status() & pros::c::E_IMU_STATUS_CALIBRATING)) {
          break;
        }
        if (iter >= 3000) {
          printf("No IMU plugged in, (took %d ms to realize that)\n", iter);
          break;
        }
      }
      pros::delay(10);
    }
    pros::c::controller_rumble(pros::E_CONTROLLER_MASTER, ".");
    printf("IMU is done calibrating (took %d ms)\n", iter);
  }
  // initialize odom
  if (odomSensors.vertical1 == nullptr)
    odomSensors.vertical1 = new lemlib::TrackingWheel(drivetrain.leftMotors, drivetrain.wheelDiameter,
                                                      -(drivetrain.trackWidth / 2), drivetrain.rpm);
  if (odomSensors.vertical2 == nullptr)
    odomSensors.vertical2 = new lemlib::TrackingWheel(drivetrain.rightMotors, drivetrain.wheelDiameter,
                                                      drivetrain.trackWidth / 2, drivetrain.rpm);
  odomSensors.vertical1->reset();
  odomSensors.vertical2->reset();
  if (odomSensors.horizontal1 != nullptr) odomSensors.horizontal1->reset();
  if (odomSensors.horizontal2 != nullptr) odomSensors.horizontal2->reset();
  lemlib::setSensors(odomSensors, drivetrain);
  lemlib::init();
  // rumble to controller to indicate success
  pros::c::controller_rumble(pros::E_CONTROLLER_MASTER, ".");
}

/**
 * @brief Creates a loading animation for the IMU. From EZ-Template
 *
 * @param iter iteration of loading screen
 */
void lemlib::Chassis::imu_loading_display(int iter) {
  // If the lcd is already initialized, don't run this function
  if (pros::lcd::is_initialized()) return;

  // Boarder
  int boarder = 50;

  // Create the boarder
  pros::screen::set_pen(COLOR_WHITE);
  for (int i = 1; i < 3; i++) {
    pros::screen::draw_rect(boarder + i, boarder + i, 480 - boarder - i, 240 - boarder - i);
  }

  // While IMU is loading
  if (iter < 2000) {
    static int last_x1 = boarder;
    pros::screen::set_pen(0x00FF6EC7);  // EZ Pink
    int x1 = (iter * ((480 - (boarder * 2)) / 2000.0)) + boarder;
    pros::screen::fill_rect(last_x1, boarder, x1, 240 - boarder);
    last_x1 = x1;
  }
  // Failsafe time
  else {
    static int last_x1 = boarder;
    pros::screen::set_pen(COLOR_RED);
    int x1 = ((iter - 2000) * ((480 - (boarder * 2)) / 1000.0)) + boarder;
    pros::screen::fill_rect(last_x1, boarder, x1, 240 - boarder);
    last_x1 = x1;
  }
}

/**
 * @brief Set the Pose object
 *
 * @param x new x value
 * @param y new y value
 * @param theta new theta value
 * @param radians true if theta is in radians, false if not. False by default
 */
void lemlib::Chassis::setPose(double x, double y, double theta, bool radians) {
  lemlib::setPose(lemlib::Pose(x, y, theta), radians);
}

/**
 * @brief Set the pose of the chassis
 *
 * @param Pose the new pose
 * @param radians whether pose theta is in radians (true) or not (false). false by default
 */
void lemlib::Chassis::setPose(Pose pose, bool radians) { lemlib::setPose(pose, radians); }

/**
 * @brief Get the pose of the chassis
 *
 * @param radians whether theta should be in radians (true) or degrees (false). false by default
 * @return Pose
 */
lemlib::Pose lemlib::Chassis::getPose(bool radians) { return lemlib::getPose(radians); }

/**
 * @brief Turn the chassis so it is facing the target point
 *
 * The PID logging id is "angularPID"
 *
 * @param x x location
 * @param y y location
 * @param timeout longest time the robot can spend moving
 * @param reversed whether the robot should turn in the opposite direction. false by default
 * @param maxSpeed the maximum speed the robot can turn at. Default is 200
 * @param log whether the chassis should log the turnTo function. false by default
 */
void lemlib::Chassis::turnTo(float x, float y, int timeout, bool reversed, float maxSpeed, bool log) {
  Pose pose(0, 0);
  float targetTheta;
  float deltaX, deltaY, deltaTheta;
  float motorPower;
  std::uint8_t compState = pros::competition::get_status();

  // create a new PID controller
  FAPID pid = FAPID(0, 0, angularSettings.kP, 0, angularSettings.kD, "angularPID");
  pid.setExit(angularSettings.largeError, angularSettings.smallError, angularSettings.largeErrorTimeout,
              angularSettings.smallErrorTimeout, timeout);

  // main loop
  while (pros::competition::get_status() == compState && !pid.settled()) {
    // update variables
    pose = getPose();
    pose.theta = (reversed) ? fmod(pose.theta - 180, 360) : fmod(pose.theta, 360);
    deltaX = x - pose.x;
    deltaY = y - pose.y;
    targetTheta = fmod(radToDeg(M_PI_2 - atan2(deltaY, deltaX)), 360);

    // calculate deltaTheta
    deltaTheta = angleError(targetTheta, pose.theta);

    // calculate the speed
    motorPower = pid.update(0, deltaTheta, log);

    // cap the speed
    if (motorPower > maxSpeed)
      motorPower = maxSpeed;
    else if (motorPower < -maxSpeed)
      motorPower = -maxSpeed;

    // move the drivetrain
    drivetrain.leftMotors->move(-motorPower);
    drivetrain.rightMotors->move(motorPower);

    pros::delay(10);
  }

  // stop the drivetrain
  drivetrain.leftMotors->move(0);
  drivetrain.rightMotors->move(0);
}

/**
 * @brief Move the chassis towards the target point
 *
 * The PID logging ids are "angularPID" and "lateralPID"
 *
 * @param x x location
 * @param y y location
 * @param timeout longest time the robot can spend moving
 * @param maxSpeed the maximum speed the robot can move at
 * @param reversed whether the robot should turn in the opposite direction. false by default
 * @param log whether the chassis should log the moveTo function. false by default
 */
void lemlib::Chassis::moveTo(float x, float y, int timeout, float maxSpeed, bool log) {
  Pose pose(0, 0);
  float prevLateralPower = 0;
  float prevAngularPower = 0;
  bool close = false;
  int start = pros::millis();
  std::uint8_t compState = pros::competition::get_status();

  // create a new PID controller
  FAPID lateralPID(0, 0, lateralSettings.kP, 0, lateralSettings.kD, "lateralPID");
  FAPID angularPID(0, 0, angularSettings.kP, 0, angularSettings.kD, "angularPID");
  lateralPID.setExit(lateralSettings.largeError, lateralSettings.smallError, lateralSettings.largeErrorTimeout,
                     lateralSettings.smallErrorTimeout, timeout);

  // main loop
  while (pros::competition::get_status() == compState && (!lateralPID.settled() || pros::millis() - start < 300)) {
    // get the current position
    Pose pose = getPose();
    pose.theta = std::fmod(pose.theta, 360);

    // update error
    float deltaX = x - pose.x;
    float deltaY = y - pose.y;
    float targetTheta = fmod(radToDeg(M_PI_2 - atan2(deltaY, deltaX)), 360);
    float hypot = std::hypot(deltaX, deltaY);
    float diffTheta1 = angleError(pose.theta, targetTheta);
    float diffTheta2 = angleError(pose.theta, targetTheta + 180);
    float angularError = (std::fabs(diffTheta1) < std::fabs(diffTheta2)) ? diffTheta1 : diffTheta2;
    float lateralError = hypot * cos(degToRad(std::fabs(diffTheta1)));

    // calculate speed
    float lateralPower = lateralPID.update(lateralError, 0, log);
    float angularPower = -angularPID.update(angularError, 0, log);

    // if the robot is close to the target
    if (pose.distance(lemlib::Pose(x, y)) < 7.5) {
      close = true;
      maxSpeed = (std::fabs(prevLateralPower) < 30) ? 30 : std::fabs(prevLateralPower);
    }

    // limit acceleration
    if (!close) lateralPower = lemlib::slew(lateralPower, prevLateralPower, lateralSettings.slew);
    if (std::fabs(angularError) > 25)
      angularPower = lemlib::slew(angularPower, prevAngularPower, angularSettings.slew);

    // cap the speed
    if (lateralPower > maxSpeed)
      lateralPower = maxSpeed;
    else if (lateralPower < -maxSpeed)
      lateralPower = -maxSpeed;
    if (close) angularPower = 0;

    prevLateralPower = lateralPower;
    prevAngularPower = angularPower;

    float leftPower = lateralPower + angularPower;
    float rightPower = lateralPower - angularPower;

    // ratio the speeds to respect the max speed
    float ratio = std::max(std::fabs(leftPower), std::fabs(rightPower)) / maxSpeed;
    if (ratio > 1) {
      leftPower /= ratio;
      rightPower /= ratio;
    }

    // move the motors
    drivetrain.leftMotors->move(leftPower);
    drivetrain.rightMotors->move(rightPower);

    pros::delay(10);
  }

  // stop the drivetrain
  drivetrain.leftMotors->move(0);
  drivetrain.rightMotors->move(0);
}

void lemlib::Chassis::set_drive_brake(pros::motor_brake_mode_e_t brake_type) {
  drivetrain.leftMotors->set_brake_modes(brake_type);
  drivetrain.rightMotors->set_brake_modes(brake_type);
}

void lemlib::Chassis::set_tank(int left, int right) {
  if (pros::millis() < 1500) return;

  drivetrain.leftMotors->move(left);
  drivetrain.rightMotors->move(right);
}

void lemlib::Chassis::drive(int l_stick, int r_stick, double active_brake_kp) {
  if (abs(l_stick) > JOYSTICK_THRESHOLD || abs(r_stick) > JOYSTICK_THRESHOLD) {
    set_tank(l_stick, r_stick);
    if (active_brake_kp != 0) reset_drive_sensor();
  }
  // When joys are released, run active brake (P) on drive
  else {
    set_tank((-drivetrain.leftMotors->get_positions()[0]) * active_brake_kp, (-drivetrain.rightMotors->get_positions()[0]) * active_brake_kp);
  }
}

void lemlib::Chassis::reset_drive_sensor() {
  drivetrain.leftMotors->tare_position();
  drivetrain.rightMotors->tare_position();
  if (odomSensors.horizontal1 != nullptr) {
    odomSensors.horizontal1->reset();
  }
  if (odomSensors.horizontal2 != nullptr) {
    odomSensors.horizontal2->reset();
  }
  if (odomSensors.vertical1 != nullptr) {
    odomSensors.vertical1->reset();
  }
  if (odomSensors.vertical2 != nullptr) {
    odomSensors.vertical2->reset();
  }
  if (odomSensors.imu != nullptr) {
    odomSensors.imu->set_heading(0);
    odomSensors.imu->set_rotation(0);
    odomSensors.imu->set_roll(0);
    odomSensors.imu->set_pitch(0);
    odomSensors.imu->set_yaw(0);
  }
}