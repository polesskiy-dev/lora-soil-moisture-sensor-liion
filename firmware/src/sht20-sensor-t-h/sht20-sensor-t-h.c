#include "sht20-sensor-t-h.h"

 struct sht20_settings_t {
    bool (*I2C_IsBusy)(void);
    bool (*I2C_ReadWrite)(uint16_t address, uint8_t* wrData, uint32_t wrLength, uint8_t* rdData, uint32_t rdLength);
} ;

static struct sht20_settings_t sht20_settings;

void sht20DriverInit(bool (*I2C_IsBusy)(void),
                     bool (*I2C_ReadWrite)(uint16_t address, uint8_t* wrData, uint32_t wrLength, uint8_t* rdData, uint32_t rdLength)) {
    sht20_settings.I2C_IsBusy = I2C_IsBusy;
    sht20_settings.I2C_ReadWrite = I2C_ReadWrite;
};

float readSHT20TemperatureSync(void) {
    uint8_t sht20TemperatureData[SHT20_ENV_DATA_LENGTH];
    sht20_settings.I2C_ReadWrite(SHT20_I2C_ADDR, (uint8_t[]){SHT20_T_MEASURE_WITH_HOLD_CMD}, SHT20_CMD_LENGTH, sht20TemperatureData, SHT20_ENV_DATA_LENGTH);
    // TODO convert to low power by calling Idle() state
    while(sht20_settings.I2C_IsBusy()) {};

    uint16_t rawT = ((uint16_t)sht20TemperatureData[0] << 8) + ((uint16_t)sht20TemperatureData[1] & 0xFC);
    float T = -46.85 + 175.72 * rawT / 0x10000; // the formula is taken from SHT20 datasheet

    return T;
};

float readSHT20RelativeHumiditySync(void) {
    uint8_t sht20RelativeHumidityData[SHT20_ENV_DATA_LENGTH];
    sht20_settings.I2C_ReadWrite(SHT20_I2C_ADDR, (uint8_t[]){SHT20_RH_MEASURE_WITH_HOLD_CMD}, SHT20_CMD_LENGTH, sht20RelativeHumidityData, SHT20_ENV_DATA_LENGTH);
    // TODO convert to low power by calling Idle() state
    while(sht20_settings.I2C_IsBusy()) {};

    uint16_t rawRH = ((uint16_t)sht20RelativeHumidityData[0] << 8) + ((uint16_t)sht20RelativeHumidityData[1] & 0xFC);
    float RH = -6 + 125 * rawRH / 0x10000; // the formula is taken from SHT20 datasheet

    return RH;
};