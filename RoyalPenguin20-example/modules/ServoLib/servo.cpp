#include "servo.hpp"

ServoDriver::ServoDriver()
{
    pwmcfg = {1000000, 20000, NULL, NULL, 0}; // 1MHz PWM clock frequency | PWM period 20 milliseconds | no callback | channel configuration set dynamicall in servoInit()
}

void ServoDriver::config()
{
    servo_arr[0] = {GPIOB, 4, &PWMD3, 0, 1000, 2000};
    servo_arr[1] = {GPIOB, 5, &PWMD3, 1, 1000, 2000};
    servo_arr[2] = {GPIOB, 6, &PWMD4, 0, 1000, 2000};
    servo_arr[3] = {GPIOB, 7, &PWMD4, 1, 1000, 2000};
    servo_arr[4] = {GPIOB, 8, &PWMD4, 2, 1000, 2000};
    servo_arr[5] = {GPIOB, 9, &PWMD4, 3, 1000, 2000};
    servo_arr[6] = {GPIOE, 14, &PWMD1, 3, 1000, 2000};
    servo_arr[7] = {GPIOE, 13, &PWMD1, 2, 1000, 2000};
    servo_arr[8] = {GPIOE, 11, &PWMD1, 1, 1000, 2000};
    servo_arr[9] = {GPIOE, 9, &PWMD1, 0, 1000, 2000};
    servo_arr[10] = {GPIOB, 1, &PWMD3, 3, 1000, 2000};
    servo_arr[11] = {GPIOB, 0, &PWMD3, 2, 1000, 2000};

    for (int i = 0; i < 12; i++)
    {
        palSetPadMode(servo_arr[i].port, servo_arr[i].pin, PAL_MODE_ALTERNATE(2));
        servoInit(&servo_arr[i]);
    }
}

void ServoDriver::servoInit(Servo_t *servo)
{
    /* create the channel configuration */
    PWMChannelConfig chcfg = {PWM_OUTPUT_ACTIVE_HIGH, NULL};
    pwmcfg.channels[servo->pwm_channel] = chcfg;

/* configure PAL */
#if defined(STM32F4XX)
    palSetPadMode(servo->port, servo->pin, PAL_MODE_ALTERNATE(2));
#else
#error "PAL configuration for this device not implemented yet - Please modify servoInit()"
#endif

    /* start the PWM unit */
    pwmStart(servo->pwm_driver, &pwmcfg);
}

void ServoDriver::servoSetValueLL(Servo_t *servo, uint16_t value)
{
    if (value > servo->max)
        value = servo->max;
    if (value < servo->min)
        value = servo->min;
    pwmEnableChannel(servo->pwm_driver, servo->pwm_channel, (pwmcnt_t)value);
}

/*!
 * @brief set servo value
 * @param n servo number (0-11)
 * @param value pwm value (default 1000-2000). It is safe to set values ​​outside the operating range.
 */
void ServoDriver::setServoValue(int n, uint16_t value)
{
    servoSetValueLL(&servo_arr[n], value);
}

uint16_t ServoDriver::getServoMaxValue(int n)
{
    return (servo_arr[n].max);
}

uint16_t ServoDriver::getServoMinValue(int n)
{
    return (servo_arr[n].min);
}