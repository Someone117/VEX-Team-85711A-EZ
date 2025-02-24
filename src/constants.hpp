// ports
#define LEFT_BACK 18
#define LEFT_FRONT 12
#define RIGHT_BACK 20
#define RIGHT_FRONT 14
#define LEFT_MIDDLE 0
#define RIGHT_MIDDLE 0
#define IMU 17
#define INTAKE 19
#define CATA 11
#define LIMIT 'b'
#define WINGS 'a'
#define POT 'c'

// chassis
#define JOYSTICK_THRESHOLD 15

// auto drive
#define DRIVE_SPEED 100  // This is 110/127 (around 87% of max speed).  We don't suggest making this 127.
                         // If this is 127 and the robot tries to heading correct, it's only correcting by
                         // making one side slower.  When this is 87%, it's correcting by making one side
                         // faster and one side slower, giving better heading correction.
#define TURN_SPEED 90
#define SWING_SPEED 90

// teleop drive
#define FORWARD_FACTOR 0.75
#define TURN_FACTOR 0.6
#define TURBO_FORWARD_FACTOR 1.0
#define TURBO_TURN_FACTOR 0.8

// catapult
#define CATA_THRESHOLD 2250
#define CATAVOLTAGE -80
#define CATAMAXVOLTAGE -127
#define CATAHOLDVOLTAGE 0
