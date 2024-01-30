#include "ch.hpp"
#include "hal.h"
#include "rt_test_root.h"
#include "oslib_test_root.h"

#include "modules/CppExampleLib/cpp_example.hpp"
#include "modules/DebugLib/print_lib.hpp"
#include "modules/MatrixLib/matrix.hpp"

#include "modules/MPU9250Lib/mpu9250.hpp"
#include "modules/MPU9250Lib/acceldata.hpp"
#include "modules/MPU9250Lib/gyrodata.hpp"

#include "servo.hpp"

#include "stdio.h"

#include <stdlib.h>
#include <string>
#include <stdio.h>

ServoDriver sd;


// #define MPU9250_ADDRESS 0x68
// #define WHO_AM_I_RESP 0x73
// #define WHO_AM_I 0x75
// #define READ_FLAG 0x80
// #define READHEX 0x00

#define SPI_BaudRatePrescaler_2 ((uint16_t)0x0000)   //  42 MHz      21 MHZ
#define SPI_BaudRatePrescaler_4 ((uint16_t)0x0008)   //  21 MHz      10.5 MHz
#define SPI_BaudRatePrescaler_8 ((uint16_t)0x0010)   //  10.5 MHz    5.25 MHz
#define SPI_BaudRatePrescaler_16 ((uint16_t)0x0018)  //  5.25 MHz    2.626 MHz
#define SPI_BaudRatePrescaler_32 ((uint16_t)0x0020)  //  2.626 MHz   1.3125 MHz
#define SPI_BaudRatePrescaler_64 ((uint16_t)0x0028)  //  1.3125 MHz  656.25 KHz
#define SPI_BaudRatePrescaler_128 ((uint16_t)0x0030) //  656.25 KHz  328.125 KHz
#define SPI_BaudRatePrescaler_256 ((uint16_t)0x0038) //  328.125 KHz 164.06 KHz

float kalman_filter(float& x_prev_k, float& P_prev_k, float zk){
  float Fk = 1;
  float Hk = 1;
  float Rk = 0.003;
  float Qk = Rk*5;
  float xk, Pk, yk, Sk, Kk;

  xk = Fk*x_prev_k;
  Pk = Fk*P_prev_k*Fk + Qk;

  yk = zk - Hk*xk;
  Sk = Hk*Pk*Hk + Rk;
  Kk = Pk*Hk*(1/Sk);
  xk = xk + Kk*yk;
  Pk = (1 - Kk*Hk)*Pk;

  x_prev_k = xk;
  P_prev_k = Pk;
}

static THD_WORKING_AREA(waThread1, 256);
static THD_FUNCTION(Thread1, arg)
{
  MPU9250 *mpu = (MPU9250 *)arg;

  float Px_prev_k = 0, Py_prev_k = 0, Pz_prev_k = 0;
  float ax = 0, ay = 0, az = 0;
  while (true)
  {
    // Accel data
    AccelData accelData;
    mpu->getAccelData(accelData);
    // Gyro data
    GyroData gyroData;
    mpu->getGyroData(gyroData);

    // SWO Print
    kalman_filter(ax, Px_prev_k, accelData.getAccelX());
    kalman_filter(ay, Py_prev_k, accelData.getAccelY());
    kalman_filter(az, Pz_prev_k, accelData.getAccelZ());

    float gx = gyroData.getAngularVelX();
    float gy = gyroData.getAngularVelY();
    float gz = gyroData.getAngularVelZ();

    printDouble(az);

    // uint16_t servo_value = 1000 + (uint16_t)(500.0 * az);
    uint16_t servo_value = sd.getServoMinValue(0) + (uint16_t)((float)((sd.getServoMaxValue(0) - sd.getServoMinValue(0)) / 2) * ax);
    sd.setServoValue(4, servo_value);

    chThdSleepMilliseconds(2);
  }
}

/*
 * Application entry point.
 */
int main(void)
{

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /*
   * Activates the serial driver 2 using the driver default configuration.
   */
  sdStart(&SD2, NULL);

  sd.config();

  /*
   * SPI configuration
   */
  SPIConfig spiConfig = {false, false, NULL, NULL, GPIOA, 4, SPI_BaudRatePrescaler_8, 0};

  /*
   * SPI1 I/O pins setup.
   */
  palSetPadMode(GPIOA, 5, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);    /* New SCK.     */
  palSetPadMode(GPIOA, 6, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);    /* New MISO.    */
  palSetPadMode(GPIOA, 7, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);    /* New MOSI.    */
  palSetPadMode(GPIOA, 4, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST); /* New CS.      */
  palSetPadMode(GPIOA, 2, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST); // ms5611 mute
  palSetPad(GPIOA, 4);
  palSetPad(GPIOA, 2);

  // MPU9250 class initialization
  MPU9250 mpu9250(&SPID1, &spiConfig);

  /*
   * Creates new thread
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO + 1, Thread1, &mpu9250);

  /*
   * Normal main() thread activity, in this demo it does nothing
   */
  while (true)
  {
    chThdSleepMilliseconds(500);
  }
  return 0;
}