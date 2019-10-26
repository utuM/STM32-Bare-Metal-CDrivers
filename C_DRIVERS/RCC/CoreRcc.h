/**
  * @file    CoreRcc.h
  * @author  utuM (Kostyantyn Komarov)
  * @version 1.0.0a
  * @date    26.10.19
  * @brief   System core frequency settings up driver.
  *          Provides setting up system frequency using PLL source only. User
  *          can select preffered frequency to be clocked by using one of the
  *          possible values from the 'CoreClock' enumeration.
  *          Also, driver provides provides master clock output (MCO) 
  *          initialization for clock frequency checking reason. To select MCO
  *          clock source need to use one from the 'CoreMcoSource' enumeration
  *          value.
  *          Current version of driver support only 8MHz external crystal.
  *          List of function:
  *          void Rcc_InitCore(CoreClock) - basic system clock frequency
  *          initialization function, call it firstly before further managing;
  *          bool Rcc_GetSystemClockInit() - get system clock setting up flag;
  *          uint32_t Rcc_GetSystemClock() - get current system clock frequency;
  *          void Rcc_InitMco(CoreMcoSource) - MCO initialization function;
  *          bool Rcc_DeInitMco() - MCO disabling function.
  *          In the driver there are a few global variables that should be
  *          changed. These are:
  *          DEFAULT_XTAL_FREQUENCY - HSE frequency that is used as a external
  *          source for PLL clocks;
  *          MCO_ENABLED - MCO feature enabling; by default, MCO could be
  *          initialized and system clock frequency will be generated using
  *          some pin, according to Reference Manual of selected MCU model; to
  *          disable it, define 'MCO_ENABLED' as 0.
  *          List of supported MCU models:
  *          + STM32F103xB.
  *          Other models will be added in future.
  **/

#ifndef __CORE_RCC_H
#define __CORE_RCC_H

#include <stdbool.h>

#include "stm32f103xb.h"

#define    DEFAULT_XTAL_FREQUENCY    8000000U ///< In MHz, need to change, if 
                                              ///  external crystal generates
                                              ///  other frequency.
#define    MCO_ENABLED    1 ///< Disable in case when MCO shouldn't be in usage.

/**
  * Possible system clock.
  * According to one of this value the target system clock should be
  * recalculated.
  **/
typedef enum
{
#if defined(STM32F103xB)
    kClock8MHz   = 0x00, ///<  8MHz clock.
    kClock16MHz,         ///< 16MHz clock.
    kClock24MHz,         ///< 24MHz clock.
    kClock32MHz,         ///< 32MHz clock.
    kClock40MHz,         ///< 40MHz clock.
    kClock48MHz,         ///< 48MHz clock.
    kClock56MHz,         ///< 56MHz clock.
    kClock64MHz,         ///< 64MHz clock.
    kClock72MHz          ///< 72MHz clock.
#else
    #error "Error: no clock list for selected MCU model."
#endif // STM32F103xB
} CoreClock;

#if (MCO_ENABLED == 1)
/**
  * Possible master clock output source.
  **/
typedef enum
{
#if defined(STM32F103xB)
    kMcoSysClkSource = 0x04, ///< System clock.
    kMcoHsiClkSource,        ///< HSI clock.
    kMcoHseClkSource,        ///< HSE clock.
    kMcoPllBy2Source,        ///< PLL clock divided by 2.
    kMcoNone         = 0xFF  ///< Default value, cannot be used.
#else
    #error "Error: no MCO source list for selected MCU model."
#endif // STM32F103xB
} CoreMcoSource;
#endif // MCO_ENABLED == 1

/**
  * System information structure.
  **/
typedef struct
{
    bool          m_isSysClkReady; ///< System clock ready flag.
    uint32_t      m_clock;         ///< Current clock value.
    bool          m_isMcoEnabled;  ///< MCO enabling flag.
#if (MCO_ENABLED == 1)
    CoreMcoSource m_mcoSource;     ///< Current MCO source.
#endif // MCO_ENABLED == 1
} SystemInfo;

#ifdef __cplusplus
extern "C"
{
#endif

void     Rcc_InitCore(CoreClock clock);

bool     Rcc_GetSystemClockInit(void);
uint32_t Rcc_GetSystemClock(void);

#if (MCO_ENABLED == 1)
bool     Rcc_InitMco(CoreMcoSource source);
bool     Rcc_DeInitMco(void);
#endif // MCO_ENABLED == 1

#ifdef __cplusplus
}
#endif

#endif // __CORE_RCC_H