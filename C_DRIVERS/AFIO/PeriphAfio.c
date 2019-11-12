/**
  * @file    PeriphAfio.c
  * @author  utuM (Kostyantyn Komarov)
  * @version 1.0.0a
  * @date    12.11.19
  * @brief   Alternative function input/output (AFIO) driver.
  *          Provides microcontroller pins enabling, disabling for the 
  *          hadrware interfaces available in selected microcontroller model.
  *          To setting up preffered pin need to enter port name, where pin is
  *          located, pin number in the selected port and mode in which pin
  *          should work. All port clock generating enabling/disabling provides
  *          by this driver.
  *          Driver provides no pin signal level getting or setting: just
  *          setting up and disabling processes that useful for interfaces. To
  *          setting up GPIO pin more wider use the 'PeriphGpio' driver.
  *          List of functions:
  *          void Afio_Init(AfioPort, AfioNumber, AfioType) - AFIO pin enabling;
  *          void Afio_DeInit(AfioPort, AfioNumber) - AFIO pin disabling.
  *          The driver has got no definitions that could be setting up.
  *          List of supported MCU models:
  *          + STM32F103xB.
  *          Other models will be added in future.
  **/

#include "PeriphAfio.h"

static void Afio_EnableClock(AfioPort port);
static GPIO_TypeDef* Afio_GetPort(AfioPort port);

/**
  * @brief  AFIO port clock generating enabling through RCC.
  * @param  port - AFIO group to be clocked.
  * @retval None.
  **/
static void Afio_EnableClock(AfioPort port)
{
    // Decide which port should be clocked.
    switch (port)
    {
#if defined(STM32F103xB)
        case kAfioA:
            RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
            break;

        case kAfioB:
            RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
            break;

        case kAfioC:
            RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
            break;

        case kAfioD:
            RCC->APB2ENR |= RCC_APB2ENR_IOPDEN;
            break;
#endif // STM32F103xB
    }
}

/**
  * @brief  Get pointer to the AFIO port structure.
  * @param  port - AFIO group to be used.
  * @retval Pointer to the GPIO structure object to be used internal by driver.
  **/
static GPIO_TypeDef* Afio_GetPort(AfioPort port)
{
    // Check preffered port.
    switch (port)
    {
#if defined(STM32F103xB)
        case kAfioA:
            return GPIOA;

        case kAfioB:
            return GPIOB;

        case kAfioC:
            return GPIOC;

        case kAfioD:
            return GPIOD;
#endif // STM32F103xB
    }

    return 0; // To prevent compiler warning.
}

/**
  * @brief  AFIO pin configuring.
  * @param  port  - GPIO pin port.
  * @param  pin   - number of the GPIO pin.
  * @param  type  - pin functionality role.
  * @retval None.
  **/
void Afio_Init(AfioPort port, AfioNumber pin, AfioType type)
{
    GPIO_TypeDef* l_pPort = Afio_GetPort(port); ///< Pointer to GPIO structure
                                                ///  to be used.

    // Enable port clock generating.
    Afio_EnableClock(port);
    // Calculate bit offset for GPIO setting up.
    uint8_t l_modeBitIndex = 4 * ((uint8_t)pin % 8);
    uint8_t l_cnfBitIndex  = l_modeBitIndex + 2;
    // Pin setting up pin from GPIO0 to GPIO7.
    if (pin <= kAfio7)
    {
        // Reset mode and configuration bits.
        l_pPort->CRL &= ~(0x03 << l_modeBitIndex);
        l_pPort->CRL &= ~(0x03 << l_cnfBitIndex);
        // Setting up output speed and turn pin into alternative function type.
        if (type >= kAfioPushPull && type <= kAfioOpenDrain)
        {
            l_pPort->CRL |= 0x03 << l_modeBitIndex;
            l_pPort->CRL |= (uint8_t)type << l_cnfBitIndex;
        }
        // Setting up input pin.
        else
        {
            if (type == kAfioInputNoPull)
            {
                l_pPort->CRL |= (uint8_t)0x01U << l_cnfBitIndex;            
            }
            else
            {
                l_pPort->ODR &= ~((uint32_t)0x01 << (uint8_t)pin);
                l_pPort->ODR |= (0x01 & (uint8_t)type) << (uint8_t)pin;
            }
        }
    }
    // Pin setting up pin from GPIO8 to GPIO15.
    else
    {
        // Reset mode and configuration bits.
        l_pPort->CRH &= ~(0x03 << l_modeBitIndex);
        l_pPort->CRH &= ~(0x03 << l_cnfBitIndex);
        // Setting up output speed and turn pin into alternative function type.
        if (type >= kAfioPushPull && type <= kAfioOpenDrain)
        {
            l_pPort->CRH |= 0x03 << l_modeBitIndex;
            l_pPort->CRH |= (uint8_t)type << l_cnfBitIndex;
        }
        // Setting up input pin.
        else
        {
            if (type == kAfioInputNoPull)
            {
                l_pPort->CRH |= (uint8_t)0x01U << l_cnfBitIndex;            
            }
            else
            {
                l_pPort->ODR &= ~((uint32_t)0x01 << (uint8_t)pin);
                l_pPort->ODR |= (0x01 & (uint8_t)type) << (uint8_t)pin;
            }
        }
    }
}

/**
  * @brief  AFIO pin disabling.
  * @param  port  - GPIO pin port.
  * @param  pin   - number of the GPIO pin.
  * @retval None.
  **/
void Afio_DeInit(AfioPort port, AfioNumber pin)
{
    GPIO_TypeDef* l_pPort = Afio_GetPort(port); ///< Pointer to GPIO structure
                                                ///  to be used.

    // Enable port clock generating.
    Afio_EnableClock(port);
    // Calculate bit offset for GPIO setting up.
    uint8_t l_modeBitIndex = 4 * ((uint8_t)pin % 8);
    uint8_t l_cnfBitIndex  = l_modeBitIndex + 2;
    // Pin setting up pin from GPIO0 to GPIO7.
    if (pin <= kAfio7)
    {
        // Reset mode and configuration bits.
        l_pPort->CRL &= ~(0x03 << l_modeBitIndex);
        l_pPort->CRL &= ~(0x03 << l_cnfBitIndex);        
    }
    else
    {
        // Reset mode and configuration bits.
        l_pPort->CRH &= ~(0x03 << l_modeBitIndex);
        l_pPort->CRH &= ~(0x03 << l_cnfBitIndex);        
    }
}