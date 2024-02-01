#ifndef DRIVERS_MS5611_HPP_
#define DRIVERS_MS5611_HPP_

#include "ch.hpp"
#include "hal.h"

class MS5611
{
public:
    MS5611(SPIDriver *spiDriver, SPIConfig *spiConfig);
    int32_t temperature;
    int32_t pressure;
    float altitude;

    void calculate();

private:
    SPIDriver *mspiDriver;
    SPIConfig *mspiConfig;

    uint16_t prom[6];

    void selectSpiDevice();
    void unSelectSpiDevice();
    void spi_write(uint8_t data);
    uint16_t spi_read_16bits(uint8_t reg);
    uint32_t spi_read_24bits(uint8_t reg);

    uint32_t readRawTemp();
    uint32_t readRawPressure();
};

#endif