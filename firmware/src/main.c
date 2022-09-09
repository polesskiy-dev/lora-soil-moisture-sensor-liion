/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "definitions.h"                // SYS function prototypes
#include "peripheral/port/plib_port.h"

#include "sht20-sensor-t-h/sht20-sensor-t-h.h" // TODO check and convert to library
#include "../libraries/sam-rtc-utils/sam-rtc-utils.X/sam-rtc-utils.h"
#include "../libraries/microchip-LoRa-semtech/src/LoRa.h"

#define DEBUG_BUF_SIZE                      512

void init(void);
static void onLoRaReceive(uint32_t packetSize);
static void handleRTCAlarm(RTC_CLOCK_INT_MASK intCause, uintptr_t context);
static void handleTC3PeriodInterrupt(TC_TIMER_STATUS intCause, uintptr_t context);
static void i2cEventHandler(uintptr_t contextHandle);
static bool SERCOM0_SPI_IsBusy(void);
static bool SERCOM0_SPI_IsTransmitterBusy(void);
//static void spiEventHandler(uintptr_t contextHandle);

static volatile bool isRTCAlarm =               false;
static volatile bool isTC3Interrupt =           false;

// TODO think how pack it properly
void _SS_CSN_LoRa_Clear(void);
void _SS_CSN_LoRa_Set(void);
void _RESET_LoRa_Clear(void);
void _RESET_LoRa_Set(void);

static float T = 0;
static float RH = 0;

#ifdef __DEBUG
    char debug_buf[2 * DEBUG_BUF_SIZE];
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    init();

    while ( true )
    {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks ( );
        if (isRTCAlarm) {
//            STATUS_LED_Toggle();
            isRTCAlarm = false;
        };

        if (isTC3Interrupt) {
            STATUS_LED_Clear();
            isTC3Interrupt = false;
            // read temperature & humidity
            T = readSHT20TemperatureSync();
            RH = readSHT20RelativeHumiditySync();

            // send LoRa packet
            // send packet
//            LoRaBeginPacket(false);
//            char buf[16];
//            sprintf(buf+2, "hello %d\r\n", counter);
//            buf[0] = 0x00;
//            buf[1] = 0x14; // LoRa address        
//            LoRaWrite((uint8_t*)buf, strlen(buf) + 1);
//            LoRaPrint(counter);
//            LoRaEndPacket(false);

            #ifdef __DEBUG
                sprintf((debug_buf + (strlen(debug_buf) % DEBUG_BUF_SIZE)),"%.02f %.02f, ", T, RH);
            #endif
            STATUS_LED_Set();
        };
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}

void init(void) {
    STATUS_LED_Clear(); // signalize init start
    struct tm initialTime;
    struct tm everyMinute = {.tm_sec = 1};
    getCurrentTimeTm(&initialTime);
    RTC_RTCCTimeSet(&initialTime);
    RTC_RTCCAlarmSet(&everyMinute, RTC_ALARM_MASK_SS);
    // SHT20 sensor
    sht20DriverInit(SERCOM3_I2C_IsBusy, SERCOM3_I2C_WriteRead);
    // LoRa
    LoRaInitDriver(
            SERCOM0_SPI_WriteRead,
            SERCOM0_SPI_Write,
            SERCOM0_SPI_Read,
            SERCOM0_SPI_IsBusy,
            SERCOM0_SPI_IsTransmitterBusy,
            _SS_CSN_LoRa_Clear,
            _SS_CSN_LoRa_Set,
            _RESET_LoRa_Clear,
            _RESET_LoRa_Set,
            SYSTICK_DelayMs
            );
    LoRaSetOnReceive(onLoRaReceive);
    LoRaBegin(434E6);
    // register interrupt callbacks
    EIC_CallbackRegister(EIC_PIN_3, LoRaHandleDio0Rise, 0); // LoRa DIO0 Rise
    RTC_RTCCCallbackRegister(handleRTCAlarm, 0);
    TC3_TimerCallbackRegister(handleTC3PeriodInterrupt, 0);
    SERCOM3_I2C_CallbackRegister(i2cEventHandler, 0);
    // SERCOM0_SPI_CallbackRegister(spiEventHandler, 0);
    // start timer
    TC3_Timer16bitPeriodSet(2 * 1024 * 10); // 10s: 2 ticks per ms * 1024 (ms in s) * 10 s
    TC3_TimerStart();
    STATUS_LED_Set(); // signalize init end
    #ifdef __DEBUG
        sprintf((debug_buf + (strlen(debug_buf) % DEBUG_BUF_SIZE)),"%s", asctime(&initialTime));
    #endif
};

void _SS_CSN_LoRa_Clear(void) {SS_CSN_LoRa_Clear();};
void _SS_CSN_LoRa_Set(void) {SS_CSN_LoRa_Set();};
void _RESET_LoRa_Clear(void) {RESET_LoRa_Clear();};
void _RESET_LoRa_Set(void) {RESET_LoRa_Set();};

static void onLoRaReceive(uint32_t packetSize) {
    if (packetSize == 0) return;          // if there's no packet, return

    // read packet header bytes:
    uint8_t recipient = LoRaRead();          // recipient address
    uint8_t sender = LoRaRead();            // sender address
    uint8_t incomingMsgId = LoRaRead();     // incoming msg ID
    uint8_t incomingLength = LoRaRead();    // incoming msg length

    char incoming[64] = "";                 // payload of packet

    while (LoRaAvailable()) {
        char ch = (char)LoRaRead();
        strncat(incoming, &ch, 1);    // can't use readString() in callback, so add bytes one by one
    };

    if (incomingLength != strlen(incoming)) {   // check length for error
        #ifdef __DEBUG
                sprintf((debug_buf + (strlen(debug_buf) % DEBUG_BUF_SIZE)),"%s", "error: message length does not match length");
        #endif
        return;                             // skip rest of function
    }

    // if the recipient isn't this device or broadcast,
//    if (recipient != localAddress && recipient != 0xFF) {
//        Serial.println("This message is not for me.");
//        return;                             // skip rest of function
//    }

    // if message is for this device, or broadcast, print details:
    #ifdef __DEBUG
        sprintf((debug_buf + (strlen(debug_buf) % DEBUG_BUF_SIZE)),
                "%s %04x %s %d %s %d %s %s %s %ul %s %ul \r\n",
                "Received from:",
                sender,
                "Message ID:",
                incomingMsgId,
                "Message length:",
                incomingLength,
                "Message:",
                incoming,
                "RSSI:",
                LoRaPacketRssi(),
                "Snr:",
                LoRaPacketSnr()
                );
    #endif
};

static void handleRTCAlarm(RTC_CLOCK_INT_MASK intCause, uintptr_t context) {
    if (intCause == RTC_CLOCK_INT_MASK_ALARM) {
        isRTCAlarm = true;
    }
};

static void handleTC3PeriodInterrupt(TC_TIMER_STATUS intCause, uintptr_t context) {
    if (intCause) {
        isTC3Interrupt = true;
    }
};

static void i2cEventHandler(uintptr_t contextHandle) {
    if (SERCOM3_I2C_ErrorGet() == SERCOM_I2C_ERROR_NONE) {
    }
};

// in SPI synchronous mode we don't need it
static bool SERCOM0_SPI_IsBusy(void) {
  return false;  
};

// in SPI synchronous mode we don't need it
static bool SERCOM0_SPI_IsTransmitterBusy(void) {
    return false;
};

//static void spiEventHandler(uintptr_t contextHandle) {
//};

/*******************************************************************************
 End of File
*/

