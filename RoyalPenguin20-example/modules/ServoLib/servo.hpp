#pragma once
#include "ch.hpp"
#include "hal.h"

struct Servo_t
{
    ioportid_t port;          // The GPIO port on which the servo is connected
    ioportmask_t pin;         // The GPIO pin on which the servo is connected
    PWMDriver *pwm_driver;    // The PWM driver to use
    pwmchannel_t pwm_channel; // The PWM channel to use
    uint16_t min;             // The minimum value the servo shall never go below
    uint16_t max;             // The maximum value the servo shall never go over
};

class ServoDriver
{
private:
    PWMConfig pwmcfg;
    void servoInit(Servo_t *servo);
    Servo_t servo_arr[12];
    void servoSetValueLL(Servo_t *servo, uint16_t value);

public:
    ServoDriver();
    void config();
    void setServoValue(int n, uint16_t value);
    uint16_t getServoMaxValue(int n);
    uint16_t getServoMinValue(int n);
};