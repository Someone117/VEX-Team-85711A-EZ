#include "main.h"

#include "..\include\constants.hpp"
#include "EZ-Template/auton.hpp"
#include "EZ-Template/auton_selector.hpp"
#include "autons.hpp"
#include "lemlib/api.hpp"
#include "pros/adi.h"
#include "pros/adi.hpp"
#include "pros/error.h"
#include "pros/misc.h"
#include "pros/misc.hpp"
#include "pros/motors.h"
#include "pros/motors.hpp"
#include "pros/rtos.hpp"

pros::Motor lF(LEFT_FRONT, pros::E_MOTOR_GEARSET_06);  // reverse as needed
pros::Motor lM(LEFT_MIDDLE, pros::E_MOTOR_GEARSET_06);
pros::Motor lB(LEFT_BACK, pros::E_MOTOR_GEARSET_06);
pros::Motor rF(RIGHT_FRONT, pros::E_MOTOR_GEARSET_06);
pros::Motor rM(RIGHT_MIDDLE, pros::E_MOTOR_GEARSET_06);
pros::Motor rB(RIGHT_BACK, pros::E_MOTOR_GEARSET_06);

// motor groups
pros::MotorGroup leftMotors({lF, lM, lB});   // left motor group
pros::MotorGroup rightMotors({rF, rM, rB});  // right motor group

// Inertial Sensor on port 6
pros::Imu imu(6);

// tracking wheels
// pros::ADIEncoder verticalEnc('A', 'B', false);  // ToDo: fix
// vertical tracking wheel. 2.75" diameter, 2.2" offset
// lemlib::TrackingWheel vertical(&verticalEnc, 2.75, 0);

// drivetrain
lemlib::Drivetrain_t drivetrain{
    &leftMotors,
    &rightMotors,
    10,
    3.25,
    360,
};

// lateral motion controller
lemlib::ChassisController_t lateralController{
    10,
    30,
    1,
    100,
    3,
    500,
    20};

// angular motion controller
lemlib::ChassisController_t angularController{
    2,
    10,
    1,
    100,
    3,
    500,
    3};

// sensors for odometry
lemlib::OdomSensors_t sensors{
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    &imu};

lemlib::Chassis chassis(drivetrain, lateralController, angularController, sensors);

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
  pros::delay(
      500);  // Stop the user from doing anything while legacy ports configure.
  // Configure your chassis controls
  // Autonomous Selector using LLEMU
  auton_selector.add_autons({Auton("AWP\n\nStart for autoAttack on defense side with triball", awp),
                             Auton("Auto Attack\n\nStart in farthest full starting tile, facing the center of the field", autoAttack),
                             Auton("Auto Defense\n\nStart in closest tile, touching the match load area, no triball",
                                   autoDefense),
                             Auton("disabled", auto_disabled),
                             Auton("Auto Skills\n\nSetup like autoDefense, with triballs galore", autoSkills)});

  pros::ADIDigitalOut wings_initializer(WINGS, LOW);

  pros::Motor inake_initializer(INTAKE, pros::E_MOTOR_GEARSET_18, false,
                                pros ::E_MOTOR_ENCODER_DEGREES);
  pros::Motor cata_initializer(CATA, pros::E_MOTOR_GEARSET_36, true);
  inake_initializer.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);
  cata_initializer.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);  // TODO: test

  // Initialize chassis and auton selector
  chassis.calibrate();
  auton_selector.initialize();
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {
  // . . .
}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {
  // . . .
}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous() {
  chassis.reset_drive_sensor();                       // Reset drive sensors, including the gyro to 0
  chassis.set_drive_brake(pros::E_MOTOR_BRAKE_HOLD);  // Set motors to hold.  This helps
                                                      // autonomous consistency.

  auton_selector
      .call_selected_auton();  // Calls selected auton from autonomous selector.
}

void arcade_standard2(bool reverse) {
  bool is_tank = false;
  chassis.reset_drive_sensor();

  int fwd_stick, turn_stick;
  // Put the joysticks through the curve function
  fwd_stick = lemlib::curve_function(master.get_analog(ANALOG_LEFT_Y), 3);
  turn_stick = lemlib::curve_function(master.get_analog(ANALOG_RIGHT_X), 3);

  turn_stick = -turn_stick;
  if (reverse)
    fwd_stick = -fwd_stick;

  if (!master.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) {  // turbo mode
    fwd_stick *= FORWARD_FACTOR;
    turn_stick *= TURN_FACTOR;
  } else {
    fwd_stick *= TURBO_FORWARD_FACTOR;
    turn_stick *= TURBO_TURN_FACTOR;
  }

  chassis.drive(fwd_stick + turn_stick, fwd_stick - turn_stick, 0.1);
}

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol() {
  chassis.set_drive_brake(pros::E_MOTOR_BRAKE_COAST);

  bool flipDrive = false;
  bool state = LOW;  // wings state
  pros::ADIDigitalOut wings(WINGS);
  // pros::ADIDigitalIn limit_switch(LIMIT);
  // pros::ADIAnalogIn pot(POT);  // do we still need this

  pros::Motor intake(INTAKE);
  pros::Motor cata(CATA);

  bool cataDown = false;  // do we need this either

  /*
   * The while loop needs to run every 10 ms so we don't screw up the drive.
   * So we have these delays, which stop the respective parts from executing to act as a debounce.
   * This should help with consistency.
   */
  unsigned int delayWings = 0;
  unsigned int delayFlip = 0;
  while (true) {
    // drive
    arcade_standard2(flipDrive);  // Standard split arcade ++

    // wings
    if (delayWings) {
      delayWings--;
    } else if (master.get_digital(pros::E_CONTROLLER_DIGITAL_A)) {
      state = !state;
      wings.set_value(state);
      delayWings = 40;
    }

    // cata
    // cataDown = limit_switch.get_value();
    // cataDown = pot.get_value() > CATA_THRESHOLD;  // we are using the limit switch

    if (master.get_digital(pros::E_CONTROLLER_DIGITAL_R1)) {
      cata = CATAMAXVOLTAGE;  // fire and continuous fire
    } else {
      cata.brake();  // coast up
    }
    // else {
    //   if (cataDown) {
    //     cata.brake();  // cata is in position to shoot
    //     lemlib::print_to_screen("Down: " + std::to_string(pot.get_value()), 0);
    //   } else {
    //     cata = CATAVOLTAGE;  // cata is going down
    //   }
    // }

    // intake
    if (master.get_digital(pros::E_CONTROLLER_DIGITAL_L2)) {
      intake = 127;
    } else if (master.get_digital(pros::E_CONTROLLER_DIGITAL_L1)) {
      intake = -127;
    } else {
      intake.brake();
    }

    // filpDrive
    if (!delayFlip) {
      if (master.get_digital(pros::E_CONTROLLER_DIGITAL_B)) {
        flipDrive = !flipDrive;
        delayFlip = 40;
      }
    } else {
      delayFlip--;
    }

    pros::delay(10);  // This is used for timer calculations!
                      // Keep this ez::util::DELAY_TIME
  }
}
