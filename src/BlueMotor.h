#pragma once

class BlueMotor
{
public:
    BlueMotor();
    void setEffort(float effort);
    void moveTo(long target);
    long getPosition();
    void reset();
    void setup();
    void setEffortWithoutDB(float effort);

private:
    void setEffort(int effort, bool clockwise);
    //static void isr();
    static void EncoderAISR();
    static void EncoderBISR();
    static int currEncPos;
    const int tolerance = 3;
    const int PWMOutPin = 11;
    const int AIN2 = 4;
    const int AIN1 = 13;
    const int ENCA = 0;
    const int ENCB = 1;
};
