#include <Arduino.h>
#include <Chassis.h>

void lineFollow(float baseSpeedLeft, float baseSpeedRight);
//void lineFollowBack(int baseSpeedLeft, int baseSpeedRight);

bool meetIntersection(void);

bool AmeetIntersection(void);

void fixTurnRight(int baseSpeedLeft, int baseSpeedRight);
void fixTurnLeft(int baseSpeedLeft, int baseSpeedRight);

void turnRight(int baseSpeedLeft, int baseSpeedRight);
void turnLeft(int baseSpeedLeft, int baseSpeedRight);