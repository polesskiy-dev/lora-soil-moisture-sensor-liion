# Soil moisture LoRa sensor (low power)
## Based on Microchip SAMD21E17D MCU

### Hardware includes:
* SAMD21E17D MCU
* LoRa Ra-02 module (Semtech SX1278)
* AT25DF SPI Flash memory
* SHT21 temperature/humidity I2C sensor
* 2x3.7 LiIon batteries
* MAX1626 DC-DC

### Sensor functionality:
* [x] RTC
* [ ] Measure temperature/humidity
* [ ] Measure Battery voltage
* [ ] Log data in SPI Flash
* [ ] Transfer data through LoRa
* [ ] Fetch commands through LoRa
  * [ ] transfer environment data
  * [ ] adjust RTC
  * [ ] transfer logs
* [ ] Low power - sleep between operations
* [ ] Wake up by RTC alarm
* [ ] Wake up by LoRa Interrupt
* [ ] Debug LED self-test

### Installation

Project can be opened in MPLAB-X, XC32 compiler

Project includes loadable libraries - *Makefile* has `.install` script for it
