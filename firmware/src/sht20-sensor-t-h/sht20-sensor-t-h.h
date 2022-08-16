//
// Created by Artem Polisskyi on 16.08.2022.
//
/**
 * @file:   sht20-sensor-t-h.h
 * @author: apolisskyi
 *
 * @date August 16, 2022
 *
 * @brief Microchip SAM devices SHT20 library. Tested on ATSAMD21E17D microcontroller, XC32 compiler.
 * @see https://sensirion.com/media/documents/CCDE1377/617A8CB7/Sensirion_Humidity_Sensors_SHT20_Datasheet.pdf
 */

#ifndef SHT20_SENSOR_T_H_H
#define SHT20_SENSOR_T_H_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define SHT20_I2C_ADDR                      0x80 >> 1
#define SHT20_CMD_LENGTH                    0x01
#define SHT20_ENV_DATA_LENGTH               0x02

#define SHT20_T_MEASURE_WITH_HOLD_CMD       0xE3
#define SHT20_RH_MEASURE_WITH_HOLD_CMD      0xE5
#define SHT20_T_MEASURE_NO_HOLD_CMD         0xF3
#define SHT20_RH_MEASURE_NO_HOLD_CMD        0xF5

/**
 * @brief Init driver for SHT20 by passing I2C communication functions
 *
 * @note properly initialized I2C bus expected
 * @note Don't forget to prevent I2C idle timeout, SHT20 will hold SCL low during measurement. For ATSAMD21 + Harmony3 it's SERCOM_I2CM_CTRLA_INACTOUT_DISABLED in SERCOMx_REGS->I2CM.SERCOM_CTRLA register
 *
 * @param I2C_IsBusy
 * @param I2C_ReadWrite
 */
void sht20DriverInit(bool (*I2C_IsBusy)(void),
                     bool (*I2C_ReadWrite)(uint16_t address, uint8_t* wrData, uint32_t wrLength, uint8_t* rdData, uint32_t rdLength));

/**
 * @brief Read SHT20 Temperature synchronously by using read-with-hold command
 * @note It takes 66-85ms to read 14bit T value
 * @note SCL line will be hold low during measurement
 *
 * @return temperature in Celsius
 * @retval 26.1234
 */
float readSHT20TemperatureSync(void);

/**
 * @brief Read SHT20 Relative Humidity synchronously by using read-with-hold command
 * @note It takes 22-29ms to read 12bit RH value
 * @note SCL line will be hold low during measurement
 *
 * @return Relative Humidity in %
 * @retval 50.1234
 */
float readSHT20RelativeHumiditySync(void);

#ifdef	__cplusplus
}
#endif

#endif //SHT20_SENSOR_T_H_H
