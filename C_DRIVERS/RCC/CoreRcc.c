/**
  * @file    CoreRcc.c
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

#include "CoreRcc.h"

static SystemInfo s_mcu =   ///< System clock object.
{
    .m_isSysClkReady = false,
    .m_clock         = DEFAULT_XTAL_FREQUENCY,
    .m_isMcoEnabled  = false
#if (MCO_ENABLED == 1)
    ,
    .m_mcoSource     = kMcoNone
#endif // MCO_ENABLED == 1
};

/**
  * @brief  RCC initialization.
  * @param  clock - preffered clock value.
  * @retval None.
  * TODO: provide system clock re-initialization.
  **/
void Rcc_InitCore(CoreClock clock)
{
    uint32_t l_tempClock = 0x00; ///< Temporary clock value for calculations.
    
#if defined(STM32F103xB)
    // Enable external crystal oscillator as a source and wait for ready state.
    RCC->CR |= RCC_CR_HSEON;
    RCC->CR |= RCC_CR_HSEBYP;
    while (!(RCC->CR & RCC_CR_HSERDY));
    // Internal flash clock.
    FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY;
    // Disable PLL.
    RCC->CR &= ~RCC_CR_PLLON;
    // Set AHB prescaler: not divided.
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;
    // Set APB1 and APB2 prescalers: APB1 - set divider to maximal one according
    // to required system clock, APB2 - not divided.
    if (clock >= kClock32MHz)
    {
        RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;
    }
    else
    {
        RCC->CFGR |= RCC_CFGR_PPRE1_DIV1;
    }
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;
    // Reset some PLL settings.
    RCC->CFGR &= ~RCC_CFGR_PLLSRC;
    RCC->CFGR &= ~RCC_CFGR_PLLMULL;
    RCC->CFGR &= ~RCC_CFGR_PLLXTPRE;
    // Setting up the HSE prescaler.
    if (clock == kClock8MHz)
    {
        // HSE prescaler: divided by 2.
        RCC->CFGR |=  RCC_CFGR_PLLXTPRE;
    }
    else
    {
        // HSE prescaler: not divided.
        RCC->CFGR &= ~RCC_CFGR_PLLXTPRE;
    }
    // Calculate temporary clock value.
    l_tempClock = (uint32_t)clock;
    if (clock >= kClock16MHz)
    {
        // Clock offset for 16MHz and above should be decreased by 1 to apply
        // properly settings for PLL multiplier: for 8MHz and 16MHz multiplier
        // should be the same.
        -- l_tempClock;
    }
    // Setting up the PLL multiplier.
    RCC->CFGR |= (l_tempClock << RCC_CFGR_PLLMULL_Pos);
    // Setting up the PLL source.
    RCC->CFGR |= RCC_CFGR_PLLSRC;
    // Enable PLL and wait before it is ready.
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));
    // Switch system source clock to PLL.
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;                     
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_1);
    // Save configured parameters as system information.
    if (clock == kClock8MHz)
    {
        s_mcu.m_clock = DEFAULT_XTAL_FREQUENCY;
    }    
    else
    {
		uint32_t l_basicFreq = DEFAULT_XTAL_FREQUENCY / 1000000U;
        s_mcu.m_clock = (2 * l_basicFreq + l_basicFreq *
                         (uint8_t)(clock - kClock16MHz)) * 1000000U;
    }
#else
    #error "Error: system clock couldn't be initialized for this MCU model."
#endif // STM32F103xB

    s_mcu.m_isSysClkReady = true;
}

/**
  * @brief  Master clock output (MCO) initialization.
  * @param  source - preffered clock source.
  * @retval true  - MCO is initialized properly.
  *         false - MCO was initialized earlier.
  **/
bool Rcc_InitMco(CoreMcoSource source)
{
#if (MCO_ENABLED == 1)
    // Check if MCO is already in usage.
    if (s_mcu.m_isMcoEnabled)
    {
        return false;
    }
    // Setting up the pin to be used as MCO.
#if defined(STM32F103xB)
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    GPIOA->CRH &= ~GPIO_CRH_CNF8;
    GPIOA->CRH |= (0x03 << GPIO_CRH_MODE8_Pos); // 50MHz.
    GPIOA->CRH |= (0x02 << GPIO_CRH_CNF8_Pos);  // Alt. function, push-pull.
    // Setting up the MCO source.
    RCC->CFGR |= (uint32_t)source << RCC_CFGR_MCO_Pos;
#else
    #error "Error: MCO couldn't be enabled because MCU model is not supported."
#endif // STM32F103xB
    // Save configured values.
    s_mcu.m_isMcoEnabled = true;
    s_mcu.m_mcoSource = source;
#endif // MCO_ENABLED == 1										
    
    return true;
}

/**
  * @brief  System clock initialization flag.
  * @param  None.
  * @retval true  - system clock is initialized.
  *         false - MCU works with default clock frequency.
  **/
bool Rcc_GetSystemClockInit(void)
{
    return s_mcu.m_isSysClkReady;
}

/**
  * @brief  System clock reading.
  * @param  None.
  * @retval 0       - system clock is not initialized properly.
  *         above 0 - current system clock.
  **/
uint32_t Rcc_GetSystemClock(void)
{
    return s_mcu.m_clock;
}

/**
  * @brief  Master clock output (MCO) de-initialization.
  * @param  None.
  * @retval true  - MCO is de-initialized properly.
  *         false - MCO was not initialized yet.
  **/
bool Rcc_DeInitMco(void)
{
#if (MCO_ENABLED == 1)
    // Check if MCO is not in usage.
    if (!s_mcu.m_isMcoEnabled)
    {
        return false;
    }
    // Reset the mode of set up GPIO.
    GPIOA->CRH &= ~GPIO_CRH_CNF8;
    GPIOA->CRH |= (0x01 << GPIO_CRH_CNF8_Pos); // Floating input (reset state).
    // Save configured values.
    s_mcu.m_isMcoEnabled = false;
    s_mcu.m_mcoSource = kMcoNone;
#endif // MCO_ENABLED == 1
    
    return true;
}