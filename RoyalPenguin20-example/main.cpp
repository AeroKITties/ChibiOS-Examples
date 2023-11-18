/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.hpp"
#include "hal.h"
#include "rt_test_root.h"
#include "oslib_test_root.h"
#include "modules/CppExampleLib/cpp_example.hpp"
#include "modules/DebugLib/print_lib.hpp"

#include "modules/MPU9250Lib/mpu9250.hpp"
#include "modules/MPU9250Lib/acceldata.hpp"
#include "modules/MPU9250Lib/gyrodata.hpp"

#include "stdio.h"

using namespace chibios_rt; // FIXME DELETE IF NOT USED

#define MPU9250_ADDRESS 0x68
#define WHO_AM_I_RESP 0x73
#define WHO_AM_I 0x75
#define READ_FLAG 0x80
#define READHEX 0x00

#define SPI_BaudRatePrescaler_2 ((uint16_t)0x0000)   //  42 MHz      21 MHZ
#define SPI_BaudRatePrescaler_4 ((uint16_t)0x0008)   //  21 MHz      10.5 MHz
#define SPI_BaudRatePrescaler_8 ((uint16_t)0x0010)   //  10.5 MHz    5.25 MHz
#define SPI_BaudRatePrescaler_16 ((uint16_t)0x0018)  //  5.25 MHz    2.626 MHz
#define SPI_BaudRatePrescaler_32 ((uint16_t)0x0020)  //  2.626 MHz   1.3125 MHz
#define SPI_BaudRatePrescaler_64 ((uint16_t)0x0028)  //  1.3125 MHz  656.25 KHz
#define SPI_BaudRatePrescaler_128 ((uint16_t)0x0030) //  656.25 KHz  328.125 KHz
#define SPI_BaudRatePrescaler_256 ((uint16_t)0x0038) //  328.125 KHz 164.06 KHz

void spiCallback(void)
{
  palTogglePad(GPIOE, 15U);
}

/*
 * Green LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg)
{

  // (void)arg;
  // chRegSetThreadName("blinker");
  MPU9250 *mpu = (MPU9250 *)arg;
  while (true)
  {
    // Accel data
    AccelData accelData;
    mpu->getAccelData(accelData);
    // Gyro data
    GyroData gyroData;
    mpu->getGyroData(gyroData);

    // SWO Print
    float ax = accelData.getAccelX();
    float ay = accelData.getAccelY();
    float az = accelData.getAccelZ();

    float gx = gyroData.getAngularVelX();
    float gy = gyroData.getAngularVelY();
    float gz = gyroData.getAngularVelZ();

    int a = 1;

    // stream << "Accel ";
    // stream << "X : " << accelData.getAccelX();
    // stream << "\n";
    // printString(stream.str().c_str());

    // BaseSequentialStream *chp = (BaseSequentialStream *)(&SDU1);
    // char buff[128];
    // sprintf(buff, "Accel [X : %.3f, Y : %.3f, Z : %.3f]\n", accelData.getAccelX(),
    //         accelData.getAccelY(),
    //         accelData.getAccelZ());
    // sprintf(buff, "1234\n");
    // printString(buff);
    //  chprintf(chp, "Gyro [X : %.3f, Y : %.3f, Z : %.3f, timestamp: %d]\n", gyroData.getAngularVelX(),
    //           gyroData.getAngularVelY(),
    //           gyroData.getAngularVelZ(),
    //           gyroData.getTimeStamp());
    chThdSleepMilliseconds(20);
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
  System::init();             // FIXME DELETE IF NOT USED
  chibios_rt::System::init(); // FIXME DELETE IF NOT USED
  chSysInit();

  /*
   * Activates the serial driver 2 using the driver default configuration.
   */
  sdStart(&SD2, NULL);

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

  // Initialize the MPU9250 class
  MPU9250 mpu9250(&SPID1, &spiConfig);

  /*
   * Creates the blinker thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO + 1, Thread1, &mpu9250);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  while (true)
  {
    // if (!palReadPad(GPIOC, GPIOC_BUTTON))
    // {
    //   test_execute((BaseSequentialStream *)&SD2, &rt_test_suite);
    //   test_execute((BaseSequentialStream *)&SD2, &oslib_test_suite);
    // }
    chThdSleepMilliseconds(500);
  }
  return 0;
}
