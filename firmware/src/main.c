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
#include "sht20-sensor-t-h/sht20-sensor-t-h.h" // TODO check and convert to library
#include "libraries/sam-rtc-utils/sam-rtc-utils.X/sam-rtc-utils.h"

#define DEBUG_BUF_SIZE                      256

void init(void);
static void handleRTCAlarm(RTC_CLOCK_INT_MASK intCause, uintptr_t context);
static void handleTC3PeriodInterrupt(TC_TIMER_STATUS intCause, uintptr_t context);
static void i2cEventHandler(uintptr_t contextHandle);

static volatile bool isRTCAlarm =               false;
static volatile bool isTC3Interrupt =           false;

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
            STATUS_LED_Toggle();
            isTC3Interrupt = false;
            // read temperature & humidity
            T = readSHT20TemperatureSync();
            RH = readSHT20RelativeHumiditySync();

            #ifdef __DEBUG
                sprintf((debug_buf + (strlen(debug_buf) % DEBUG_BUF_SIZE)),"%.02f %.02f, ", T, RH);
            #endif
        };
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}

void init(void) {
    STATUS_LED_Clear(); // signalize init start
//    time_t rawtime = time(0);
//    static struct tm initialTime = {.tm_sec = 1, .tm_min = 1, .tm_hour = 1, .tm_mday = 1, .tm_mon = 1, .tm_year = 2022};// = localtime(&t); // get local time
//    struct tm* initialTime = localtime(&rawtime); // get local time
    struct tm initialTime;
    getCurrentTimeTm(&initialTime);
    struct tm everyMinute = {.tm_sec = 1};
    RTC_RTCCTimeSet(&initialTime);
    RTC_RTCCAlarmSet(&everyMinute, RTC_ALARM_MASK_SS);
    // SHT20 sensor
    sht20DriverInit(SERCOM3_I2C_IsBusy, SERCOM3_I2C_WriteRead);
    // register interrupt callbacks
    RTC_RTCCCallbackRegister(handleRTCAlarm, 0);
    TC3_TimerCallbackRegister(handleTC3PeriodInterrupt, 0);
    SERCOM3_I2C_CallbackRegister(i2cEventHandler, 0);
    // start timer
    TC3_Timer16bitPeriodSet(2 * 1024 * 10); // 10s: 2 ticks per ms * 1024 (ms in s) * 10 s
    TC3_TimerStart();
    STATUS_LED_Set(); // signalize init end
    #ifdef __DEBUG
        sprintf((debug_buf + (strlen(debug_buf) % DEBUG_BUF_SIZE)),"%s", asctime(&initialTime));
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
}

//void getCurTimeTm (struct tm* curTimeTm) {
//    const char* curTime = __TIME__;
//    const char* curDate = __DATE__;
//    char month[4];
//    
//    #ifdef __DEBUG
//        sprintf((debug_buf + (strlen(debug_buf) % DEBUG_BUF_SIZE)),"%s %s", curTime, curDate);
//    #endif
//
//    const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug",
//                                 "Sep", "Oct", "Nov", "Dec"};
//    sscanf(curDate,"%s %d %d",month,&curTimeTm->tm_mday,&curTimeTm->tm_year);
//    curTimeTm->tm_year-=1900U;
//    sscanf(curTime,"%d:%d:%d",&curTimeTm->tm_hour,&curTimeTm->tm_min,&curTimeTm->tm_sec);
//
//    for (uint8_t i = 0; i < 12; i++)
//        if (!strcmp(month, months[i])) {
//            curTimeTm->tm_mon = i;
//            return;
//        };
//};

/*******************************************************************************
 End of File
*/

