#include <Arduino.h>
#include "BlueMotor.h"
#include <Romi32U4.h>
#include "Timer.h"

int oldValue = 0;
int newValue;
int count = 0;
unsigned time = 0;

float blueKp = 30;
float blueKi = 30;

static uint32_t lastError = 0;

//int printDelay(500);
//Timer printTimer(printDelay);

BlueMotor::BlueMotor()
{
}

void BlueMotor::setup()
{
    pinMode(PWMOutPin, OUTPUT);
    pinMode(AIN2, OUTPUT);
    pinMode(AIN1, OUTPUT);
    pinMode(ENCA, INPUT);
    pinMode(ENCB, INPUT);
    TCCR1A = 0xA8; //0b10101000; //gcl: added OCR1C for adding a third PWM on pin 11
    TCCR1B = 0x11; //0b00010001;
    ICR1 = 400;
    OCR1C = 200;


    //UPDATED TO USE QUADRATURE
    attachInterrupt(digitalPinToInterrupt(ENCA), EncoderAISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCB), EncoderBISR, CHANGE);
    reset();
}

long BlueMotor::getPosition()
{
    return count;
}

void BlueMotor::reset()
{
    count = 0;
}


void BlueMotor::EncoderAISR()
{
    if (digitalRead(0) == digitalRead(1)){
        count++;

    }
    else count--;
}
void BlueMotor::EncoderBISR()
{
    if (digitalRead(0) == digitalRead(1)){
        count --;
    }
    else count++;
}

void BlueMotor::setEffort(float effort)
{
    if (effort < 0)
    {
        setEffort(-effort, true);
    }
    else
    {
        setEffort(effort, false);
    }
}

void BlueMotor::setEffort(int effort, bool clockwise)
{
    if (clockwise)
    {
        digitalWrite(AIN1, HIGH);
        digitalWrite(AIN2, LOW);
    }
    else
    {
        digitalWrite(AIN1, LOW);
        digitalWrite(AIN2, HIGH);
    }
    OCR1C = constrain(effort, 0, 400);
}

// METHOD CREATED FOR SECTION 3
void BlueMotor::setEffortWithoutDB(float effort)
{
    // account for dead band
    float newEffort = abs(150 + abs(0.625 * effort));

    if (newEffort > 420)
    {
        // if the newEffort is greater than the max effort, set newEffort to max effort
        newEffort = 420;
    }  
    if (effort < 0)
    {
        setEffort(newEffort, true);
    }
    else
    {
        setEffort(newEffort, false);
    }
}


// METHOD CREATED FOR SECTION 2
//Move to this encoder position within the specified
//tolerance in the header file using proportional control
//then stop
void BlueMotor::moveTo(long target)
{
    float desiredPosition = (540.00/360.00) * target;
    reset();
    int currPosition = getPosition();

    // 
    while(desiredPosition != currPosition)
    {
        // get and the current position and desired position
        Serial.print("DESIRED POSITION   ");
        Serial.println(desiredPosition);
        currPosition = getPosition();
        Serial.print("CURRENT POSITION   ");
        Serial.println(currPosition);
        delay(100);

        // motor will stop if the current position is within range of desired position
        if(abs(desiredPosition) - abs(currPosition) < 10)
        {
            setEffort(0);
            Serial.println("POSITION REACHED");
            break;
        }

        // if the desired position is negative, the effort will set using proportional control
        if (desiredPosition < 0)
        {
            float error =  desiredPosition - currPosition;
            float deltaError = lastError + error;
            float effort = blueKp * error + blueKi * deltaError;  
            setEffortWithoutDB(effort);  
            
            lastError = error;             
        }

        // if the desired position is positive, the effort will be set using proportional control
        if(desiredPosition > 0)
        {
            float error =  desiredPosition - currPosition;
            float deltaError = -lastError - error; 
            float effort = blueKp * error + blueKi * deltaError;    
            setEffortWithoutDB(-effort);
  
            lastError = error;    
        }
      
    }
    reset();
}
