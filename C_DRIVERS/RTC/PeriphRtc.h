/**
  * @file    PeriphRtc.h
  * @author  utuM (Kostyantyn Komarov)
  * @version 1.0.0a
  * @date    17.11.19
  * @brief   Real-time clock (RTC) driver.
  *          Provides logic to manage real-time separated timer for time
  *          counting. The timer could be initialized with different clock
  *          generating source using 'RtcSource' value.
  *          After timer initial process, the interruption event flags will be
  *          toggled every 1 second. In this case, driver will process these
  *          flags and increase own counter by 1. The counter presents total
  *          amount of seconds that already counted that could be reset or
  *          defined by other amount.
  *          Also, RTC enabling/disabling state could be managed by this driver.
  *          List of functions:
  *          bool Rtc_Init(const RtcSource, const bool) - driver enabling
  *          function;
  *          bool Rtc_Toggle(const bool) - RTC temporary enabling/disabling;
  *          bool Rtc_SetCounter(const uint64_t) - setting up preferred amount
  *          of seconds already counted;
  *          bool Rtc_Reset(void) - driver counter value reseting;
  *          bool Rtc_DeInit(void) - driver disabling function.
  *          In the driver there are a few global variables that could be
  *          changed. These are:
  *          DEFAULT_RTC_PRIORITY - preferred priority value of the 
  *          interruption event; could be changed up by 0 to 3;
  *          DEFAULT_SECONDS_VALUE - default reset value for the driver counter
  *          value.
  *          List of supported MCU models:
  *          + STM32F103xB.
  *          Other models will be added in future.
  **/

#ifndef __PERIPHERAL_RTC_H
#define __PERIPHERAL_RTC_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "stm32f103xb.h"
#include "CoreRcc.h"

/**
  * Possible clock generating source.
  **/
typedef enum
{
    kLseSource  = 0x01U, ///< LSE oscillator clock source.
    kLsiSource,          ///< LSI oscillator clock source.
    kHseDiv128           ///< HSE oscillator clock divided by 128 source.
} RtcSource;

#ifdef __cplusplus
extern "C"
{
#endif
    
bool Rtc_Init(const RtcSource source, const bool isActive);
bool Rtc_Toggle(const bool isActive);
bool Rtc_SetCounter(const uint64_t value);
bool Rtc_Reset(void);
bool Rtc_DeInit(void);
    
#ifdef __cplusplus
}
#endif

#endif // __PERIPHERAL_RTC_H