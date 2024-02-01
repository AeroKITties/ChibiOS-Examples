#include "ms5611.hpp"

MS5611::MS5611(SPIDriver *spiDriver, SPIConfig *spiConfig) : mspiDriver(spiDriver), mspiConfig(spiConfig)
{
    spi_write(0x1E);
    chThdSleepMilliseconds(10);

    prom[0] = spi_read_16bits(0xA2);
    prom[1] = spi_read_16bits(0xA4);
    prom[2] = spi_read_16bits(0xA6);
    prom[3] = spi_read_16bits(0xA8);
    prom[4] = spi_read_16bits(0xAA);
    prom[5] = spi_read_16bits(0xAC);
}

void MS5611::selectSpiDevice()
{
    // Acquire SPI bus
    spiAcquireBus(mspiDriver);

    // Start SPI transaction
    spiStart(mspiDriver, mspiConfig);

    // Select the SPI device
    spiSelect(mspiDriver);
}

void MS5611::unSelectSpiDevice()
{
    // Unselect the slave
    spiUnselect(mspiDriver);

    // Release bus to be used by others
    spiReleaseBus(mspiDriver);
}

void MS5611::spi_write(uint8_t data)
{
    // Select SPI device
    this->selectSpiDevice();
    // Send data
    spiSend(mspiDriver, 1, &data);
    // Unselect and release the SPI device
    this->unSelectSpiDevice();
}

uint16_t MS5611::spi_read_16bits(uint8_t reg)
{
    uint8_t txBuffer[] = {reg};
    uint8_t rxBuffer[3];

    this->selectSpiDevice();
    spiExchange(mspiDriver, 3, &txBuffer, &rxBuffer);
    this->unSelectSpiDevice();

    uint16_t return_value = ((uint16_t)rxBuffer[1] << 8) | (rxBuffer[2]);
    return return_value;
}

uint32_t MS5611::spi_read_24bits(uint8_t reg)
{
    uint8_t txBuffer[] = {reg};
    uint8_t rxBuffer[4];

    this->selectSpiDevice();
    spiExchange(mspiDriver, 4, &txBuffer, &rxBuffer);
    this->unSelectSpiDevice();

    uint32_t return_value = ((uint32_t)rxBuffer[1] << 16) | ((uint32_t)(rxBuffer[2] << 8)) | (rxBuffer[3]);
    return return_value;
}

uint32_t MS5611::readRawTemp()
{
    uint32_t D2;
    // Convert temp
    spi_write(0x50);
    // Conversion Time
    chThdSleepMilliseconds(1);
    // Read ADC
    D2 = spi_read_24bits(0x00);

    return D2;
}

uint32_t MS5611::readRawPressure()
{
    uint32_t D1;
    // Convert pressure
    spi_write(0x40);
    // Conversion time
    chThdSleepMilliseconds(1);
    // Read ADC
    D1 = spi_read_24bits(0x00);

    return D1;
}

void MS5611::calculate()
{
    int32_t dT;
    int64_t TEMP, OFF, SENS, P;
    uint32_t D1, D2;
    float press, r, c;

    D1 = readRawPressure();
    D2 = readRawTemp();

    dT = D2 - ((long)prom[4] * 256);
    TEMP = 2000 + ((int64_t)dT * prom[5]) / 8388608;
    OFF = (int64_t)prom[1] * 65536 + ((int64_t)prom[3] * dT) / 128;
    SENS = (int64_t)prom[0] * 32768 + ((int64_t)prom[2] * dT) / 256;

    if (TEMP < 2000)
    { // second order temperature compensation
        int64_t T2 = (((int64_t)dT) * dT) >> 31;
        int64_t Aux_64 = (TEMP - 2000) * (TEMP - 2000);
        int64_t OFF2 = (5 * Aux_64) >> 1;
        int64_t SENS2 = (5 * Aux_64) >> 2;
        TEMP = TEMP - T2;
        OFF = OFF - OFF2;
        SENS = SENS - SENS2;
    }

    P = (D1 * SENS / 2097152 - OFF) / 32768;
    temperature = TEMP;
    pressure = P;

    press = (float)pressure;
    r = press / 101325.0;
    c = 1.0 / 5.255;
    // altitude = (1 - pow(r, c)) * 44330.77;
}