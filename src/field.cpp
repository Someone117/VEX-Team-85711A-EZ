/*
 * Field locations for auto
 * Units are inches and degrees
 * Field reference is in the engineering notebook
 * Blue/Red colors refer to the side of the field
 */

struct Pose2d {
  float x;
  float y;

  Pose2d(float xVal, float yVal);
};

float tile = 24;
float fieldX = 144;
float fieldY = 144;

// Field red defense left side defined as (0, 0)
Pose2d origin = Pose2d(0, 0);
Pose2d fieldCenter = Pose2d(fieldX / 2.0, fieldY / 2.0);

Pose2d blueElevationHorizontalMid = Pose2d(12, tile * 3);
Pose2d redElevationHorizontalMid = Pose2d((fieldX - tile / 2.0), tile * 3);

Pose2d blueGoalCenter = Pose2d((fieldX / 2.0), tile);
Pose2d redGoalCenter = Pose2d((fieldX / 2.0), (fieldY - tile));

Pose2d redLeftCornerTriball = Pose2d((tile / 4.0), (tile / 4.0));
Pose2d redRightCornerTriball = Pose2d((fieldX - tile / 4.0), (tile / 4.0));
Pose2d blueLeftCornerTriball = Pose2d((tile / 4.0), (fieldY - tile / 4.0));
Pose2d blueRightCornerTriball = Pose2d((fieldX - tile / 4.0), (fieldY - tile / 4.0));
Pose2d blueUnderElevationTriball = Pose2d((tile / 2.0), (fieldY / 2.0));
Pose2d redUnderElevationTriball = Pose2d((fieldX - tile / 2.0), (fieldY / 2.0));

// triballs near the center of the field
//Pose2d blueCenterLowerTriball = Pose2d((fieldX / 2.0), );
