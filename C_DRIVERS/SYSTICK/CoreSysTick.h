/**
  * @file    CoreSysTick.h
  * @author  utuM (Kostyantyn Komarov)
  * @version 1.0.1a
  * @date    26.10.19 (1.0.0a)
  * @date    31.10.19 (1.0.1a)
  * @brief   System timer driver.
  *          Provides setting up of the Cortex-M SysTick timer using current
  *          system clock frequency value. Driver using the 'CoreRcc' driver
  *          to read current frequency used for tick calculation.
  *          Tick calculation logic calculates what amount of ticks should be
  *          passed before the interruption is generated. Uses tick unit 
  *          (presented by 'm_stepUnit' in 'SysTickHandler') and amount of units
  *          (presented by 'm_stepSize' in 'SysTickHandler'). The first variable
  *          could be selected from the 'SysTickStep' enumeration and the second
  *          one should be defined with amount of the units. For example, when
  *          interruption should be generated every 500ms, then need to select
  *          the 'kTickMillisecs' for 'm_stepUnit' and define 'm_stepSize' with
  *          value 500.
  *          List of function:
  *          void SysTick_Init(SysTickStep, uint16_t) - basic SysTick timer
  *          initialization;
  *          uint64_t SysTick_GetTick() - get current 8-byte ticks amount;
  *          bool Rcc_GetSystemClockInit() - get system clock setting up flag;
  *          uint32_t SysTick_GetTickLength() - tick time duration;
  *          void SysTick_ResetTick() - resets current ticks amount to 0.
  *          In the driver there are a few global variables that should be
  *          changed. These are:
  *          DEFAULT_PRIORITY - SysTick interruption priority in NVIC; better
  *          not to change this value;
  *          MAXIMAL_STEP_SIZE - amount of units for units type; is the basic
  *          value the microseconds or milliseconds amount will be cut-off to;
  *          SYSTICK_SECS_DIV - basic divider for seconds; is the basic one to
  *          configure interruption generating every second;
  *          SYSTICK_MILLISECS_DIV - basic divider for milliseconds; is the
  *          basic one to configure interruption generation every millisecond.
  *          List of supported MCU models:
  *          + STM32F103xB.
  *          Other models will be added in future.
  **/

#ifndef __CORE_SYSTICK_H
#define __CORE_SYSTICK_H

#include <stdbool.h>
#include <stdint.h>

#include "CoreRcc.h"
#include "stm32f103xb.h"

/**
  * Possible step units.
  **/
typedef enum
{
    kTickMicrosecs = 0x00, ///< Microseconds.
    kTickMillisecs         ///< Milliseconds.
} SysTickStep;

/**
  * System timer handler.
  **/
typedef struct
{
    bool          m_isInit;   ///< Initialization flag.
    SysTickStep   m_stepUnit; ///< Current step unit.
    uint16_t      m_stepSize; ///< Amount of units.
    uint64_t      m_tick;     ///< Current tick value.
} SysTickHandler;

#ifdef __cplusplus
extern "C"
{
#endif

void     SysTick_Init(SysTickStep step, uint16_t units);
uint64_t SysTick_GetTick(void);
uint32_t SysTick_GetTickLength(void);
void     SysTick_ResetTick(void);

#ifdef __cplusplus
}
#endif

#endif // __CORE_SYSTICK_H