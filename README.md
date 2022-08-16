# Soil moisture LoRa sensor (low power)
## Based on Microchip SAMD21E17D MCU

### Hardware includes:
* SAMD21E17D MCU
* LoRa Ra-02 module (Semtech SX1278)
* AT25DF SPI EEPROM memory
* SHT20 environment temperature/humidity I2C sensor
* 2x3.7 LiIon batteries
* MAX1626 DC-DC

### Sensor functionality:
* [x] RTC
* [ ] Measure temperature/humidity
* [ ] Measure Battery voltage
* [ ] Log data in SPI EEPROM
* [ ] Transfer environment data through LoRa
* [ ] Fetch commands through LoRa
  * [ ] transfer environment data
  * [ ] adjust RTC
  * [ ] transfer logs
  * [ ] compress and transfer logs (zLib)
* [ ] Low power consumption
  * [ ] Wake up by RTC alarm
  * [ ] Wake up by LoRa Interrupt
  * [ ] DMA I2C read
  * [ ] DMA SPI LoRa read
* [ ] Debug LED self-test

### Installation

Project can be opened in MPLAB-X, XC32 compiler

Project includes loadable libraries - *Makefile* has `.install` script for it
