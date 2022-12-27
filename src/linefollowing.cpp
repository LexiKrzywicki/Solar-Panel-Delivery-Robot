#include <Arduino.h>
#include <Chassis.h>
#include "linefollowing.h"

//value on black tape ~700
//value on white ~36
int setPoint = 150;

//pins for the line sensor
const int left_pin = A4;
const int right_pin = A3;

// Kp and Ki values for proportional and integral control
// initial Kp = 0.03
float Kp = 0.0175;
//float Kp = 0.005;
// initial ki = 0.04
float Ki = 0.024;

unsigned long lastTime = 0;

const unsigned long LINE_FOLLOWING_INTERVAL = 500; 

static uint32_t lastErrorLeft = 0;
static uint32_t lastErrorRight = 0;


// line following code
void lineFollow(float baseSpeedLeft, float baseSpeedRight)
{

  //Serial.println("LINE FOLLOWING");
  unsigned long currTime = millis();


  if (currTime - lastTime > LINE_FOLLOWING_INTERVAL)
  {
    // read the sensors
    int left_sensor_val = analogRead(left_pin);
    int right_sensor_val = analogRead(right_pin);

    //prints the pin values of the line sensor to the serial monitor
    Serial.print("LEFT   ");
    Serial.println(analogRead(left_pin));
    Serial.print("RIGHT            ");
    Serial.println(analogRead(right_pin));
    
    // sets the value to the current reading
    int currLeft = left_sensor_val;
    int currRight = right_sensor_val;

    if (currLeft > setPoint)//robot needs to turn left
    {
      //calculates the new speed for the motor based on the error
      float errorLeft = abs(setPoint - currLeft);
      float deltaErrorLeft = abs(errorLeft - lastErrorLeft);
      float effortLeft = Kp * errorLeft + Ki * deltaErrorLeft;
      //float newSpeedleft = baseSpeedRight - effortLeft;

      float newSpeedRight = baseSpeedLeft + effortLeft;

      //sets motor to the new speed
      //chassis.setMotorEfforts(newSpeedelft, baseSpeedright);
      chassis.setMotorEfforts(baseSpeedLeft, newSpeedRight);

      //set current error to the last error
      lastErrorLeft = errorLeft;

    }

    else if(currRight > setPoint) // robot needs to turn right
    {
      //calculates the new speed for the motor based on the error
      float errorRight = abs(setPoint - currRight);
      float deltaErrorRight = abs(errorRight - lastErrorRight);
      float effortRight = Kp * errorRight + Ki * deltaErrorRight;
      //float newSpeedright = baseSpeedLeft - effortRight;
      float newSpeedLeft = baseSpeedRight + effortRight;
  
      //sets motor to the new speed
      //chassis.setMotorEfforts(baseSpeedLeft, newSpeedright);
      chassis.setMotorEfforts(newSpeedLeft, baseSpeedRight);

      //sets current error to the last error
      lastErrorRight = errorRight;
    }
    else // robot drives forward
    {
      chassis.setMotorEfforts(baseSpeedLeft, baseSpeedRight);
    }

    //sets last time as the current time
    lastTime = currTime;
  }
}


bool meetIntersection(void) // checks for intersection
{
    int currLeft = analogRead(left_pin);
    int currRight = analogRead(right_pin);

    if(currLeft > 300  && currRight > 300)
    {    
      chassis.setMotorEfforts(0,0);
      delay(750);
      Serial.println("INTERSECTION MET");
      chassis.setMotorEfforts(70,65);
      delay(600);
      chassis.setMotorEfforts(0,0);
      delay(500);
      return true;
    }
    else
    {
      return false;
    }
}

bool AmeetIntersection(void) // checks for intersection
{
    int currLeft = analogRead(left_pin);
    int currRight = analogRead(right_pin);

    if(currLeft > 300  && currRight > 300)
    {    
      chassis.setMotorEfforts(0,0);
      delay(750);
      Serial.println("ALUMINUM INTERSECTION MET");
      chassis.setMotorEfforts(70,65);
      delay(1300);
      chassis.setMotorEfforts(0,0);
      delay(500);
      return true;
    }
    else
    {
      return false;
    }
}



// turns the robot to the right
void turnRight(int baseSpeedLeft, int baseSpeedRight)
{
      chassis.setMotorEfforts(baseSpeedLeft, -baseSpeedRight);
      delay(500);
      chassis.idle();
      delay(500);

      int right_sensor_val = analogRead(right_pin);
      int currRightT = right_sensor_val;
      //Serial.println(right_sensor_val);

  // runs while the right sensor is on the white surface and stops when it senses the line
  while(currRightT < setPoint)
  {
      //int left_sensor_val = analogRead(left_pin);
      int right_sensor_val = analogRead(right_pin);

      //int currLeftT = left_sensor_val;
      int currRightT = right_sensor_val;
      chassis.setMotorEfforts(baseSpeedLeft, -baseSpeedRight);  
      if(currRightT > setPoint)
      {
        chassis.idle();
        delay(1000);
        break;
      } 
  }
}

// turns the robot to the left
void turnLeft(int baseSpeedLeft, int baseSpeedRight)
{
      chassis.setMotorEfforts(-baseSpeedLeft, baseSpeedRight);
      delay(850);
      chassis.idle();
      delay(500);

      int currLeft = analogRead(right_pin);

  // runs while the right sensor is on the white surface and stops when it senses the line
  while(currLeft < setPoint)
  {
      //int left_sensor_val = analogRead(left_pin);
      int currLeft = analogRead(right_pin);

      //int currLeftT = left_sensor_val;
      //int currRightT = right_sensor_val;
      chassis.setMotorEfforts(-baseSpeedLeft, baseSpeedRight);  
      if(currLeft > setPoint)
      {
        chassis.idle();
        delay(1000);
        break;
      } 
  }
}



//aligns robot to line, used after turning right
void fixTurnRight(int baseSpeedLeft, int baseSpeedRight)
{
    int right_sensor_val = analogRead(right_pin);
    int currRightA = right_sensor_val;

    //int left_sensor_val = analogRead(left_pin);
    //int currLeft = left_sensor_val;

    //turns the robot until the right sensor is off tape
    while(currRightA > setPoint)
    {
      chassis.setMotorEfforts(baseSpeedLeft, -baseSpeedRight);
      //int left_sensor_val = analogRead(left_pin);
      int right_sensor_val = analogRead(right_pin);

      //int currLeftA = left_sensor_val;
      int currRightA = right_sensor_val;
      if (currRightA < setPoint)
      {
        chassis.idle();
        delay(500);
        Serial.println("FIXED RIGHT TURN");
        break;
      }
    }
} 

//aligns robot to line, used after turning left
void fixTurnLeft(int baseSpeedLeft, int baseSpeedRight)
{
    int currLeft = analogRead(left_pin);

    // turns the robot until the right sensor is off tape
    while(currLeft > setPoint)
    {
      chassis.setMotorEfforts(-baseSpeedLeft, baseSpeedRight);
      //int left_sensor_val = analogRead(left_pin);
      int currLeft = analogRead(right_pin);

     // int currLeftA = left_sensor_val;
      //int currRightA = right_sensor_val;
      if (currLeft < 70)
      {
        chassis.idle();
        delay(500);
        Serial.println("FIXED LEFT TURN");
        break;
      }
    }
} 