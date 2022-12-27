#include <Arduino.h>
#include <Chassis.h>
#include "linefollowing.h"
#include "Rangefinder.h"
#include "IRdecoder.h"
#include <Romi32U4.h>
#include <stdbool.h>
#include "BlueMotor.h"
#include <servo32u4.h>
#include <ir_codes.h>
//#include "contServo"

// motor value
long timeToPrint = 0;
long now = 0;
long newPosition = 0;
long oldPosition = 0;
long sampleTime = 100;
int speedInRPM = 0;
int CPR = 270;
int motorEffort = 400;
int lastPosition = 0;
// long lastTime = 0;

bool withPanel = false;

// nc servo values
int positionUp = 2400;
int positionMiddle = 2000;
int positionDown = 1300;

// define pins for the ultrasonic sensor
const uint8_t ECHO_PIN = 17;
const uint8_t TRIG_PIN = 12;

const uint8_t IR_DETECTOR_PIN = 14;

Chassis chassis;
BlueMotor motor;
Servo32U4 servo;
Romi32U4ButtonA buttonA;
Romi32U4ButtonB buttonB;
//For line follow test
Romi32U4ButtonC buttonC;
Rangefinder rangefinder(ECHO_PIN, TRIG_PIN);
IRDecoder decoder(IR_DETECTOR_PIN);

// speed for right and left motor
// pinkbot
// int baseSpeedRight = 70;
// int baseSpeedLeft = 77;

//Function Definitions

void robotStateSelect(void);
void keyPressDetect(void);

//blackbot
// old speedleft 70
float baseSpeedLeft = 55.0;
// ols speedRight 65
float baseSpeedRight = 49.0;


//pins for the line sensor
const int left_pin = A4;
const int right_pin = A3;

//int setPoint2 = 150;

//bool pastIntersection = false;

//states named according to field
//functions named relative to robot
enum ROBOT_STATE
{
  ROBOT_ACTIVE,
  ROBOT_HOUSE_LEFT,
  ROBOT_HOUSE_RIGHT,
  ROBOT_STAGE_LEFT,
  ROBOT_STAGE_RIGHT,
  ROBOT_HOUSE25_PICKUP,
  ROBOT_HOUSE25_PLACE,
  ROBOT_HOUSE45_PICKUP,
  LINE_FOLLOWING_CODE,
  ROBOT_HOUSE45_PLACE,
  ROBOT_STAGE_PLACE,
  AROUND_FROM_LEFT,
  AROUND_FROM_RIGHT
};

ROBOT_STATE robotState = ROBOT_ACTIVE;
ROBOT_STATE initialState = ROBOT_ACTIVE;
ROBOT_STATE globalState1; //used for keeping track of state for emergency stop

void setup() 
{
  chassis.init();
  chassis.setMotorPIDcoeffs(5, 0.5);
  Serial.begin(9600);
  rangefinder.init();
  decoder.init();
  motor.setup();
  motor.reset();
  servo.setMinMaxMicroseconds(500, 2500);
}

void clawClose()
{
    servo.writeMicroseconds(positionDown);
    delay(1200);
    Serial.println(analogRead(A0));
    Serial.println("CLAW CLOSE");
}

void turn90Right()
{
  chassis.setMotorEfforts(baseSpeedLeft, -baseSpeedRight);
  delay(1000);
  chassis.idle();
}

void turn90Left()
{
  chassis.setMotorEfforts(-baseSpeedLeft, baseSpeedRight);
  delay(1200);
  chassis.idle();
}

bool isPressed = false;


void loop() 
{

  //  while(isPressed == false)
  // {
    
  //   if (buttonB.isPressed())
  //   {
  //     clawClose();
  //     isPressed = true;
  //     withPanel = true;
  //   }  
  //   //robotStateSelect();
  // }
  
  robotStateSelect();

  //motor.moveTo(30);





  // while(buttonB.isPressed() == false)
  // {
    // motor.moveTo(30);
    // delay(500);
    // int currLeft = analogRead(A4);
    // int currRight = analogRead(A3);

    // Serial.print("LEFT   ");
    // Serial.println(currLeft);
    // Serial.print("RIGHT  ");
    // Serial.println(currRight);
  // }


  

  //hank: neg = away, pos = towards
  //duck: neg = towards, pos = away

  //45 degree roof from stage: moveTo(-1780)
  // stage to 20 degree roof: moveTo(-2180)
}







void clawOpen()
{
    servo.writeMicroseconds(positionUp);
    delay(1500);
    Serial.println(analogRead(A0)); 
    Serial.println("CLAW OPEN");
}

/*
Function to switch robot state based on key pressed
*/
void keyPressDetect()
{
  int16_t keyPress = decoder.getKeyCode();
  bool pressed = false;
  while (!pressed)
  {
    //Serial.println("Waiting for Key Press...");
    delay(100);
    keyPress = decoder.getKeyCode();
    
    
    if (keyPress == NUM_1) //button 1 => ROBOT_HOUSE_LEFT
    {
      robotState = ROBOT_HOUSE_LEFT;
      initialState = ROBOT_HOUSE_LEFT;
      //pastIntersection = false;
      pressed = true;
    }
    else if (keyPress == NUM_4) //button 4 => ROBOT_STAGE_LEFT
    {
      withPanel = true;
      robotState = ROBOT_STAGE_LEFT;
      initialState = ROBOT_STAGE_LEFT;
      ///pastIntersection = false;
      pressed = true;
    }
    else if (keyPress == NUM_3) //button 3 => ROBOT_HOUSE_RIGHT
    {
      robotState = ROBOT_HOUSE_RIGHT;
      pressed = true;
    }
    else if (keyPress == NUM_6) //button 6 => ROBOT_STAGE_RIGHT
    {
      withPanel = true;
      robotState = ROBOT_STAGE_RIGHT;
      pressed = true;
    }
    else if(keyPress == NUM_7)  //button 7 => drop off panel
    {
      withPanel = true;
      motor.moveTo(-720);
      initialState = ROBOT_ACTIVE;
      robotState = ROBOT_HOUSE_LEFT;
      pressed = true;
    }
    else if(keyPress == NUM_9)
    {
      withPanel = true;
      motor.moveTo(-720);//!!!
      initialState = ROBOT_ACTIVE;
      robotState = ROBOT_HOUSE_RIGHT;
      pressed = true;
    }
    else if (keyPress == NUM_0_10) //0 10+ 
    {
      robotState = AROUND_FROM_LEFT;
      pressed = true;
    }
    else if (keyPress == REWIND) // rewind
    {
      robotState = AROUND_FROM_RIGHT;
      pressed = true;
    }
    else
    {
      pressed = false;
    }
  }
  Serial.println(robotState);
}


boolean emergencyStop = false;
void robotStateSelect() 
{
  switch(robotState)
  {
    case ROBOT_ACTIVE:
    {
      Serial.println("ROBOT_ACTIVE");
      //pastIntersection = false;
      keyPressDetect();
      initialState = ROBOT_ACTIVE;
      break;
    }
    case ROBOT_HOUSE_LEFT:
    {
      while (decoder.getKeyCode() != STOP_MODE && !emergencyStop)
      {
        Serial.println("ROBOT_HOUSE_LEFT");
      //HANK GOES OPP DUCK
      //DUCK GOES NEG TOWARDS AND POS AWAY
      if(initialState == ROBOT_ACTIVE)
      {
        motor.moveTo(-1780);
        turnRight(baseSpeedLeft, baseSpeedRight);
        fixTurnRight(baseSpeedLeft, baseSpeedRight);    
        chassis.idle();
        initialState = ROBOT_HOUSE_LEFT;
      }

      lineFollow(baseSpeedLeft, baseSpeedRight);

      if(withPanel == true && AmeetIntersection() == true)
      {

          turnRight(baseSpeedLeft, baseSpeedRight);
          fixTurnRight(baseSpeedLeft, baseSpeedRight); 

          robotState = LINE_FOLLOWING_CODE;
          break;        
      }

      if(meetIntersection() == true && withPanel == false)
      {
        turnRight(baseSpeedLeft, baseSpeedRight);
        fixTurnRight(baseSpeedLeft, baseSpeedRight);   

        //pastIntersection = true; 

        robotState = LINE_FOLLOWING_CODE;
        break;
      }
      //breaks from loop because code is finished
      break;
      }
      //breaks be
      //if (the stopkey is pressed)
      {
          globalState1 = ROBOT_HOUSE_LEFT;
          robotState = globalState1;
      }
      
      break;
      
    }
    case ROBOT_STAGE_LEFT:
    {
      while (decoder.getKeyCode() != STOP_MODE)
      {
      Serial.println("ROBOT_STAGE_LEFT");
      //delay(1000);

      if(initialState == ROBOT_ACTIVE)
       {
        motor.moveTo(-1080);
        turnRight(baseSpeedLeft, baseSpeedRight);
        fixTurnRight(baseSpeedLeft, baseSpeedRight);
       }

      initialState = ROBOT_STAGE_LEFT;
      lineFollow(baseSpeedLeft, baseSpeedRight);

      if(AmeetIntersection() == true)
        {
          turnLeft(baseSpeedLeft, baseSpeedRight);
          fixTurnLeft(baseSpeedLeft, baseSpeedRight);

          //pastIntersection = true;
          //Serial.println("PASTED INTERSECTION");

          robotState = LINE_FOLLOWING_CODE;
        }

      //initialState = ROBOT_STAGE_LEFT;       
      break;    
      }
      break;
    }
    case ROBOT_HOUSE_RIGHT:
    {
      while (decoder.getKeyCode() != STOP_MODE)
      {
      //Serial.println("ROBOT_HOUSE_RIGHT");

      if(initialState == ROBOT_ACTIVE)
      {
        motor.moveTo(-2180);
        turnRight(baseSpeedLeft, baseSpeedRight);
        fixTurnRight(baseSpeedLeft, baseSpeedRight);
        initialState = ROBOT_HOUSE_RIGHT;
        break;
      }

      // moved inside if
      //initialState = ROBOT_HOUSE_RIGHT;

      lineFollow(baseSpeedLeft, baseSpeedRight);

      if(withPanel == true && AmeetIntersection() == true)
      {

          turnLeft(baseSpeedLeft, baseSpeedRight);
          fixTurnLeft(baseSpeedLeft, baseSpeedRight); 

          robotState = LINE_FOLLOWING_CODE;
          break;        
      }

      if(withPanel == false && meetIntersection() == true)
      {
        turnLeft(baseSpeedLeft, baseSpeedRight);
        fixTurnLeft(baseSpeedLeft, baseSpeedRight);   

        //pastIntersection = true; 

        robotState = LINE_FOLLOWING_CODE;  // !!! need line following with panel to house 25
        break;
      }

        break;
    }
      break;
    }
    case ROBOT_STAGE_RIGHT:
    {
      while (decoder.getKeyCode() != STOP_MODE)
      {
      Serial.println("ROBOT_STAGE_RIGHT");
      //delay(1000);
      if(initialState == ROBOT_ACTIVE)
      {
        motor.moveTo(-720);
        delay(500);
        turnRight(baseSpeedLeft, baseSpeedRight);
        fixTurnRight(baseSpeedLeft, baseSpeedRight);
      }

      initialState = ROBOT_STAGE_RIGHT;

      lineFollow(baseSpeedLeft, baseSpeedRight);

      if(AmeetIntersection() == true)
      {
        turnLeft(baseSpeedLeft, baseSpeedRight);
        fixTurnLeft(baseSpeedLeft, baseSpeedRight);

        robotState = LINE_FOLLOWING_CODE;
      }
      break;
      }
      break;
    }

    case LINE_FOLLOWING_CODE:
    {
      while (decoder.getKeyCode() != STOP_MODE)
      {
      //Serial.println("LINE_FOLLOWING_CODE");
      //delay(500);
      lineFollow(baseSpeedLeft -5 , baseSpeedRight -5);
      float distance = rangefinder.getDistance(); 

      //Serial.println(distance);
      //pastIntersection = false; 

      //doesn't have panel and is going to pick up at 45
      if(initialState == ROBOT_HOUSE_LEFT && distance < 18.5 && distance > 12 && withPanel == false)
      {
        Serial.println("ROBOT_HOUSE_LEFT WITHOUT PANEL");
        Serial.println(distance);
        chassis.idle();
        robotState = ROBOT_HOUSE45_PICKUP;
      }

      //has panel and going to drop at 45 degree house (slightly shorter distance)
      if(initialState == ROBOT_HOUSE_LEFT && distance < 14.0 && distance > 12 && withPanel == true)
      {
        Serial.println("ROBOT_HOUSE_LEFT WITH PANEL");
        Serial.println(distance);
        chassis.idle();
        robotState = ROBOT_HOUSE45_PLACE;
      }
           
      // has panel for 25 roof     
      if(initialState == ROBOT_HOUSE_RIGHT && distance < 14 && distance > 12 && withPanel == true)
      {
        Serial.println("ROBOT_HOUSE_LEFT WITH PANEL");
        Serial.println(distance);
        chassis.idle();
        robotState = ROBOT_HOUSE25_PLACE;
      }

      // going to stage from 45
      if(initialState == ROBOT_STAGE_LEFT && distance < 12 && distance > 10 && withPanel == true)
      {
        chassis.idle();
        delay(500);
        Serial.println("APPROACHED STAGE");
        //motor.moveTo(2500);

        // move closer to the stage 
        chassis.setMotorEfforts(baseSpeedLeft, baseSpeedRight);
        delay(690);
        chassis.idle();

        initialState = ROBOT_HOUSE45_PICKUP;
        robotState = ROBOT_STAGE_PLACE;
      }


      // no panel and going to house
      if(distance > 12 && distance < 18 && initialState == ROBOT_HOUSE_RIGHT && withPanel == false)
      {
        Serial.println(distance);
        chassis.idle();
        robotState = ROBOT_HOUSE25_PICKUP;
      }
      

      if(initialState == ROBOT_STAGE_RIGHT && distance < 10)
      {
        Serial.println(distance);
        chassis.idle();

        initialState = ROBOT_HOUSE25_PICKUP;
        robotState = ROBOT_STAGE_PLACE;
      }
      
      //from is goign around the house not actually line following here, but 
      if(initialState == AROUND_FROM_LEFT && meetIntersection() == true)
      {
        chassis.idle();

        turnRight(baseSpeedLeft, baseSpeedRight);
        fixTurnRight(baseSpeedLeft, baseSpeedRight);

        initialState = ROBOT_HOUSE45_PLACE;
        robotState = ROBOT_STAGE_PLACE;
      }
      break;
      }
      
      break;
      
    }

    case ROBOT_HOUSE25_PICKUP:
    {
      while (decoder.getKeyCode() != STOP_MODE)
      {
      Serial.println("ROBOT_HOUSE25_PICKUP");

      if(analogRead(A0) < positionDown)
      {
        clawOpen();
      }
      delay(800);

      // chassis.setMotorEfforts(50, 57);
      // delay(500);
      // chassis.idle();

      clawClose();

      //initialState = ROBOT_HOUSE_RIGHT;

      robotState = ROBOT_ACTIVE;
      break;
      }
      break;
    }
    case ROBOT_HOUSE25_PLACE:
    {
      while (decoder.getKeyCode() != STOP_MODE)
      {
      Serial.println("ROBOT_HOUSE25_PLACE");

      chassis.setMotorEfforts(baseSpeedLeft, baseSpeedRight);
      delay(700);
      //chassis.idle();

      //house align correction NEW!!!
      chassis.setMotorEfforts(-60, -50);
      delay(200);
      chassis.idle();

      //open the claw
      clawOpen();

      withPanel = false;      //dropped off panel

      // 360 original
      motor.moveTo(450);
      delay(500);

      //back up a little
      chassis.setMotorEfforts(-baseSpeedLeft, -baseSpeedRight);
      delay(700);
      chassis.idle();



      chassis.setMotorEfforts(-baseSpeedLeft, -baseSpeedRight);
      delay(500);
      chassis.idle();


      robotState = ROBOT_ACTIVE; 

      break;
      }

      break;
    }
    case ROBOT_HOUSE45_PICKUP:
    {
      while (decoder.getKeyCode() != STOP_MODE)
      {
      Serial.println("ROBOT_HOUSE45_PICKUP");
      //open the claw
      if(analogRead(A0) < positionDown)
      {
        clawOpen();
      }

      //wait
      //delay(500);

      //drive forward a little
      chassis.setMotorEfforts(baseSpeedLeft, baseSpeedRight);
      delay(700);
      chassis.idle();
      
      //close the claw
      clawClose();

      withPanel = true;

      robotState = ROBOT_ACTIVE;
      break;
      }
      break;
    }
    case ROBOT_HOUSE45_PLACE:
    {
      while (decoder.getKeyCode() != STOP_MODE)
      {
      Serial.println("ROBOT_HOUSE45_PLACE");

      chassis.setMotorEfforts(baseSpeedLeft, baseSpeedRight);
      delay(700);
      chassis.idle();

      //open the claw
      clawOpen();

      withPanel = false;      //dropped off panel


      //back up a little
      chassis.setMotorEfforts(-baseSpeedLeft, -baseSpeedRight);
      delay(700);
      chassis.idle();

      // 360 original
      motor.moveTo(450);
      delay(500);

      chassis.setMotorEfforts(-baseSpeedLeft, -baseSpeedRight);
      delay(700);
      chassis.idle();


      robotState = ROBOT_ACTIVE;
      break;
      }
      break;
    }
    case ROBOT_STAGE_PLACE:
    {
      while (decoder.getKeyCode() != STOP_MODE)
      {
      //Serial.println(initialState);

      if(initialState == ROBOT_HOUSE25_PICKUP)
      {
        motor.moveTo(2900);
        clawOpen();

        delay(1500);

        clawClose();
        
        robotState = ROBOT_ACTIVE;
      }

      if(initialState == ROBOT_HOUSE45_PICKUP)
      {
        chassis.idle();

        motor.moveTo(2860);
        //motor.moveTo(200);
        //motor.moveTo(360);
        clawOpen();

        delay(1500);

        clawClose();

        chassis.idle();

        withPanel = true;
        robotState = ROBOT_ACTIVE;

        break;
      }

      //if going to pickup panel
      if(initialState == ROBOT_HOUSE45_PLACE)
      {
        motor.moveTo(1780);

        //open the claw
        clawOpen();
        
        //wait for someone to put a panel in
        delay(1500);

        //close the claw
        clawClose();
        
        //go back to active state and wait for new button press
        robotState = ROBOT_ACTIVE;
      }

      if(initialState == ROBOT_HOUSE25_PLACE)
      {
        //motor.moveTo()

        clawOpen();

        delay(1500);

        clawClose();

        robotState = ROBOT_ACTIVE;
      }
      break;
      }
      break;

    }
    case AROUND_FROM_LEFT:
    {
      while (decoder.getKeyCode() != STOP_MODE)
      {
      if(initialState == ROBOT_ACTIVE)
      {
        motor.moveTo(1950);
        // turn 
        turn90Right();
        delay(500);
        // move forewards
        chassis.setMotorEfforts(baseSpeedLeft, baseSpeedRight);
        delay(2250);
        chassis.idle();
        delay(500);
        // left
        turn90Left();
        delay(500);

        initialState = AROUND_FROM_LEFT;
        break;
      }

      chassis.setMotorEfforts(baseSpeedLeft, baseSpeedRight);

      if(initialState == AROUND_FROM_LEFT && meetIntersection() == true)
      {
        chassis.idle();
        delay(1000);
        turnRight(baseSpeedLeft, baseSpeedRight);
        fixTurnRight(baseSpeedLeft, baseSpeedRight);
        chassis.idle();
        delay(500);
        chassis.setMotorEfforts(-60, -65);
        delay(1000);
        chassis.idle();
        //motor.moveTo(30);
        robotState = ROBOT_ACTIVE;
        break;
      }

      // if(initialState == AROUND_FROM_LEFT && meetIntersection() == false)
      // {
      //   chassis.setMotorEfforts(baseSpeedLeft, baseSpeedRight);
      //   break;
      // }
      break;
      }

      break;

    }
    case AROUND_FROM_RIGHT:
    {
      while (decoder.getKeyCode() != STOP_MODE)
      {
      if(initialState == ROBOT_ACTIVE)
      {
        //motor.moveTo(-450);
        // turn 
        turn90Left();
        delay(500);
        // move forewards
        chassis.setMotorEfforts(baseSpeedLeft, baseSpeedRight);
        delay(2250);
        chassis.idle();
        delay(500);
        // right
        turn90Right();
        delay(500);

        initialState = AROUND_FROM_RIGHT;
        break;
      }

      chassis.setMotorEfforts(baseSpeedLeft, baseSpeedRight);

      if(initialState == AROUND_FROM_RIGHT && meetIntersection() == true)
      {
        chassis.idle();
        delay(1000);
        turnLeft(baseSpeedLeft, baseSpeedRight);
        fixTurnLeft(baseSpeedLeft, baseSpeedRight);
        chassis.idle();
        delay(500);
        chassis.setMotorEfforts(-60, -65);
        delay(1000);
        chassis.idle();
        motor.moveTo(30);
        robotState = ROBOT_ACTIVE;
        break;
      }

      // if(initialState == AROUND_FROM_RIGHT && meetIntersection() == false)
      // {
      //   chassis.setMotorEfforts(baseSpeedLeft, baseSpeedRight);
      //   break;
      // }


      // // turn 
      // //turn90Left();
      // //move forewards
      // chassis.setMotorEfforts(baseSpeedLeft, baseSpeedRight);
      // delay(750);
      // chassis.idle();

      // initialState = AROUND_FROM_RIGHT;
      // robotState = LINE_FOLLOWING_CODE;
      break;
      }
      break;
    }
  }
}

