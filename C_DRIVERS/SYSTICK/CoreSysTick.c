/**
  * @file    CoreSysTick.c
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
  *          List of functions:
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

#include "CoreSysTick.h"

#define    DEFAULT_PRIORITY               0U ///< SysTick NVIC default priority.

#define    MAXIMAL_STEP_SIZE           1000U ///< Amount of units limiter.

#define    SYSTICK_SECS_DIV         1000000U
#define    SYSTICK_MILLISECS_DIV       1000U

/**
  * System timer handler.
  **/
typedef struct
{
    bool        m_isInit;   ///< Initialization flag.
    SysTickStep m_stepUnit; ///< Current step unit.
    uint16_t    m_stepSize; ///< Amount of units.
    uint64_t    m_tick;     ///< Current tick value.
} SysTickHandler;

static SysTickHandler s_sysTick = ///< System timer object.
{
    .m_isInit   = false,
    .m_stepUnit = kTickMillisecs,
    .m_stepSize = 1U,
    .m_tick     = 0U
};

/**
  * @brief  System timer callback.
  * @param  None.
  * @retval None.
  **/
void SysTick_Handler(void)
{
    ++ s_sysTick.m_tick;
}

/**
  * @brief  System timer initialization.
  *         Limits tick units amount to comfortable value. When milliseconds
  *         in usage, the unit amount should be equal or below 1000 milliseconds
  *         (1 seconds), or when microseconds in usage, the unit amount also 
  *         should be equal or below 1000 microseconds (1 millisecond). Also,
  *         when unit amount is 0 the value will be increased to 1.
  * @param  step  - preffered step units.
  * @param  ticks - amount of time units to increase handler ticks.
  * @retval None.
  **/
void SysTick_Init(SysTickStep step, uint16_t units)
{
    // Check if system timer is ready.
    if (s_sysTick.m_isInit)
    {
        return;
    }
    
    // Read current system clock.
    uint32_t l_ticks = 0U;                 ///< System timer ticks amount for
                                           ///  overloading.
    uint32_t l_divider = SYSTICK_SECS_DIV; ///< Divider according to time unit.

    // Calculate frequency divider.
    for (uint8_t i = 0; i < (uint8_t)s_sysTick.m_stepUnit; i ++)
    {
        l_divider /= SYSTICK_MILLISECS_DIV;
    }
    // Check if need to limit 'stepSize' value.
    if (s_sysTick.m_stepSize > MAXIMAL_STEP_SIZE)
    {
        s_sysTick.m_stepSize = MAXIMAL_STEP_SIZE;
    }
    else if (!s_sysTick.m_stepSize)
    {
        s_sysTick.m_stepSize = 1;
    }
    // Calculate reload value.
    l_ticks = Rcc_GetSystemClock() / (l_divider / s_sysTick.m_stepSize) - 1;
    // Write reload value into 'SYST_RVR' register (24-bit value only).
    SysTick->LOAD = 0x00FFFFFFU & l_ticks;
    SysTick->VAL  = 0U;
    // Set the interrupt priority.
    NVIC_SetPriority(SysTick_IRQn, (0x01 << __NVIC_PRIO_BITS) - 1);
    // Set system clock as system timer source.
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;
    // Enabled overload interruption.
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
    // Enable system timer.
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    
    s_sysTick.m_isInit = true;
}

/**
  * @brief  Tick value reading.
  * @param  None.
  * @retval 0     - system timer is not started or not configured properly.
  *         above - current tick value. 
  **/
uint64_t SysTick_GetTick(void)
{
    if (!s_sysTick.m_isInit)
    {
        return 0U;
    }
    
    return s_sysTick.m_tick;
}

/**
  * @brief  Get tick duration.
  *         Return the tick time duration after which the interruption is
  *         toggled.
  * @param  None.
  * @retval Tick time length in microseconds.
  **/
uint32_t SysTick_GetTickLength(void)
{
    // Check if system time is not ready.
    if (!s_sysTick.m_isInit)
    {
        return 0U;
    }
    // Return tick time duration.
    if (s_sysTick.m_stepUnit == kTickMicrosecs)
    {
        return s_sysTick.m_stepSize;
    }
    return (1000U * s_sysTick.m_stepSize);
}

/**
  * @brief  Reseting current tick value to 0.
  * @param  None.
  * @retval None.
  **/
void SysTick_ResetTick(void)
{
    if (!s_sysTick.m_isInit)
    {
        return;
    }
    s_sysTick.m_tick = 0U;
}