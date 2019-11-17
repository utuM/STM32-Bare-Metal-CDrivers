/**
  * @file    PeriphRtc.c
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

#include "PeriphRtc.h"

#define    DEFAULT_RTC_PRIORITY      0           ///< Default RTC interrupt
                                                 ///  priority level.
#define    DEFAULT_SECONDS_VALUE    (uint64_t)-1 ///< Default RTC handler
                                                 ///  seconds amount value.

/**
  * RTC handler structure.
  **/
typedef struct
{
    bool      m_isInit;     ///< RTC handler readiness flag.
    bool      m_isActive;   ///< RTC active state.
    RtcSource m_source;     ///< RTC clock source.
    uint64_t  m_seconds;    ///< Current tickes amount of seconds.
    bool      m_isLocked;   ///< Locking flag.
} RtcHandler;

#define    DEFAULT_RTC_HANDLER    {                          \
                                      false,                 \
                                      false,                 \
                                      kLsiSource,            \
                                      DEFAULT_SECONDS_VALUE, \
                                      false                  \
                                  }

static RtcHandler s_rtc = DEFAULT_RTC_HANDLER; ///< RTC handler.

static void Rtc_ConfigModeToggle(const bool isEntered);
void RTC_IRQHandler(void);

/**
  * @brief  RTC configuration mode entering/exiting.
  * @param  isEntered - mode entering flag.
  * @retval None.
  **/
static void Rtc_ConfigModeToggle(const bool isEntered)
{
    if (isEntered) // Enter into the RTC configuration mode.
    {
        while (!(RTC->CRL & RTC_CRL_RTOFF));
        RTC->CRL |= RTC_CRL_CNF;
    }
    else           // Exit from the RTC configuration mode.
    {
        RTC->CRL &= ~(RTC_CRL_CNF);
        while (!(RTC->CRL & RTC_CRL_RTOFF));
    }
}

/**
  * @brief  RTC interruption handler.
  * @param  None.
  * @retval None.
  **/
void RTC_IRQHandler(void)
{
    // Clean up all interruptions.
    if ((RTC->CRL & RTC_CRL_OWF) || (RTC->CRL & RTC_CRL_SECF))
    {
        RTC->CRL &= ~(RTC_CRL_OWF | RTC_CRL_SECF);
    }
    // Increase the total amount of ticked seconds.
    ++ s_rtc.m_seconds;
    printf("RTC tick is %u.\r\n", (uint32_t)s_rtc.m_seconds);
}

/**
  * @brief  RTC driver enabling.
  * @param  source   - preferred clock generation source.
  * @param  isActive - start acitvity state. 
  * @retval true  - driver is successful enabled.
  *         false - driver is already enabled.
  **/
bool Rtc_Init(const RtcSource source, const bool isActive)
{
    if (s_rtc.m_isInit)
    {
        return false;
    }
    
    uint32_t l_sourceFreq = 0U; ///< Source clock frequency for RTC.
    // Setting up Backup domain.
    RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;
    PWR->CR |= PWR_CR_DBP;
    // Enter into the configuration mode.
    Rtc_ConfigModeToggle(true);
    // Disable RTC.
    RCC->BDCR &= ~(RCC_BDCR_RTCEN);
    // Setting up the RTC clock source.
    RCC->BDCR &= ~(RCC_BDCR_RTCSEL);
    switch (source)
    {
        case kLseSource: // For 32.768kHz external oscillator.
            RCC->BDCR |= RCC_BDCR_LSEON;
            while (!(RCC->BDCR & RCC_BDCR_LSERDY));
            l_sourceFreq = 32767U;
            break;
            
        case kLsiSource: // For 40.0kHz internal oscillator.
            l_sourceFreq = 39999U;
            break;
            
        case kHseDiv128: // For HSE oscillator divided by 128.
            l_sourceFreq = DEFAULT_XTAL_FREQUENCY / 128U - 1;
            break;
    }
    RCC->BDCR |= source << RCC_BDCR_RTCSEL_Pos;
    s_rtc.m_source = source;
    // Setting up RTC prescaler load register to generate an event every 1
    // second.
    l_sourceFreq &= 0x000FFFFF;
    RTC->PRLH = (uint16_t)(l_sourceFreq >> 16);
    RTC->PRLL = (uint16_t)l_sourceFreq;
    // Clear the counter register.
    RTC->CNTH = 0U;
    RTC->CNTL = 0U;
    // Enable second and overflow interrupts and disable alarm one.
    RTC->CRH = RTC_CRH_SECIE | RTC_CRH_OWIE;
    // Enable RTC if input activity flag is toggled up.
    if (isActive)
    {
        RCC->BDCR |= RCC_BDCR_RTCEN;
        // Enable interruption.
        NVIC_SetPriority(RTC_IRQn, DEFAULT_RTC_PRIORITY);
        NVIC_EnableIRQ(RTC_IRQn);
        // Exit the configuration mode and wait before the command is done.
        Rtc_ConfigModeToggle(false);
    }
    s_rtc.m_isActive = isActive;
    
    return (s_rtc.m_isInit = true);
}

/**
  * @brief  RTC enabling/disabling.
  * @param  isActive - RTC enabling state.
  * @retval true  - enabling/disabling is done successfully.
  *         false - driver is not enabled or input activity state is equal to
  *                 the current one.
  **/
bool Rtc_Toggle(const bool isActive)
{
    if (!s_rtc.m_isInit || s_rtc.m_isActive == isActive)
    {
        return false;
    }

    // Toggle the RTC activity only when input state is not equal to the current
    // one.
    if (isActive && !s_rtc.m_isActive)
    {
        // Enabling RTC.
        Rtc_ConfigModeToggle(true);
        RCC->BDCR |= RCC_BDCR_RTCEN;
        Rtc_ConfigModeToggle(false);
        // Enable interruption.
        NVIC_SetPriority(RTC_IRQn, DEFAULT_RTC_PRIORITY);
        NVIC_EnableIRQ(RTC_IRQn);
    }
    else if (!isActive && s_rtc.m_isActive)
    {
        // Disable interruption.
        NVIC_DisableIRQ(RTC_IRQn);
        // Disabling RTC.
        Rtc_ConfigModeToggle(true);
        RCC->BDCR &= ~(RCC_BDCR_RTCEN);
        Rtc_ConfigModeToggle(false);
    }
    s_rtc.m_isActive = isActive;

    return true;
}

/**
  * @brief  Handler seconds value setting up.
  * @param  value - new seconds value.
  * @retval true  - seconds value is set up successfully.
  *         false - driver is not enabled.
  **/
bool Rtc_SetCounter(const uint64_t value)
{
    if (!s_rtc.m_isInit)
    {
        return false;
    }
    // Setting up handler seconds value using input one.
    s_rtc.m_seconds = value;
    
    return true;
}

/**
  * @brief  Handler seconds value reseting.
  * @param  None.
  * @retval true  - seconds value is reset successfully.
  *         false - driver is not enabled. 
  **/
bool Rtc_Reset(void)
{
    if (!s_rtc.m_isInit)
    {
        return false;
    }
    // Setting up handler seconds values into the default one.
    s_rtc.m_seconds = DEFAULT_SECONDS_VALUE;
    
    return true;
}

/**
  * @brief  Driver disabling.
  * @param  None.
  * @retval true  - driver is successful disabled.
  *         false - driver was not enabled.
  **/
bool Rtc_DeInit(void)
{
    if (!s_rtc.m_isInit)
    {
        return false;
    }
    
    // Disable the RTC and clean up some fields.
    Rtc_Toggle(false);
    Rtc_Reset();
    // Clean up all the handler fields values.
    RtcHandler l_tempHandler = DEFAULT_RTC_HANDLER;
    memcpy(&s_rtc, &l_tempHandler, sizeof(RtcHandler));
    
    return true;
}