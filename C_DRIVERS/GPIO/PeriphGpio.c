/**
  * @file    PeriphGpio.c
  * @author  utuM (Kostyantyn Komarov)
  * @version 1.0.0a
  * @date    31.10.19
  * @brief   General purpose input/output (GPIO) driver.
  *          Provides microcontroller pins enabling, disabling and managing.
  *          Initialization processes for any pin mode supported by some
  *          amount of setting for each mode. Input pin mode can be enabled
  *          with/without any of pull type. Output pin mode can be setting up
  *          using output type, toggling speed and default signal level, that
  *          should be generated after initialization.
  *          Driver provides reading of current input signal level and
  *          output signal level generation.
  *          Notice, that level writing function handles 8-byte type
  *          'gpio_state', but value will be limited by LSB bit of this value.
  *          So, for example, if You put the value '2' as a value of the
  *          'state' parameter of function 'Gpio_OutputWrite(...)', then
  *          driver will take only LSB bit of this value and it will be '0'.
  *          List of functions:
  *          void Gpio_InitInput(GpioPort, GpioNumber, GpioPull) - input pin
  *          initialization with port clock source enabling;
  *          void Gpio_InitOutput(GpioPort, GpioNumber, GpioOutput, GpioSpeed,
  *          gpio_state) - output pin initialization with port clock source
  *          enabling;
  *          uint8_t Gpio_InputRead(GpioPort, GpioNumber) - input signal level
  *          checking;
  *          bool Gpio_OutputWrite(GpioPort, GpioNumber, gpio_state) - output
  *          signal level generating;
  *          void Gpio_DeInit(GpioPort, GpioNumber) - input/output pin
  *          disabling without port clock source disabling.
  *          In the driver there is only one type definition:
  *          gpio_state - 8-byte type, based on the 'unsigned char', used to
  *          separate output signal level value from other types.
  *          List of supported MCU models:
  *          + STM32F103xB.
  *          Other models will be added in future.
  **/

#include "PeriphGpio.h"

static void Gpio_EnableClock(GpioPort port);
static GPIO_TypeDef* Gpio_GetPort(GpioPort port);
static bool Gpio_CheckPin(GPIO_TypeDef* pGpio, GpioNumber pin, GpioMode mode);

/**
  * @brief  GPIO port clock generating enabling through RCC.
  * @param  port - GPIO group to be clocked.
  * @retval None.
  **/
static void Gpio_EnableClock(GpioPort port)
{
    // Decide which port should be clocked.
    switch (port)
    {
#if defined(STM32F103xB)
        case kGpioA:
            RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
            break;

        case kGpioB:
            RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
            break;

        case kGpioC:
            RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
            break;

        case kGpioD:
            RCC->APB2ENR |= RCC_APB2ENR_IOPDEN;
            break;
#endif // STM32F103xB
    }
}

/**
  * @brief  Get pointer to the GPIO port structure.
  * @param  port - GPIO group to be used.
  * @retval Pointer to the GPIO structure object to be used internal by driver.
  **/
static GPIO_TypeDef* Gpio_GetPort(GpioPort port)
{
    // Check preffered port.
    switch (port)
    {
#if defined(STM32F103xB)
        case kGpioA:
            return GPIOA;

        case kGpioB:
            return GPIOB;

        case kGpioC:
            return GPIOC;

        case kGpioD:
            return GPIOD;
#endif // STM32F103xB
    }

    return 0; // To prevent compiler warning.
}

/**
  * @brief  Pin type checking.
  * @param  pGpio - pointer to the port structure where the GPIO pin is placed.
  * @param  pin   - number of the GPIO pin.
  * @param  mode  - possible GPIO pin mode.
  * @retval true  - GPIO pin mode is confirmed.
  *         false - GPIO pin mode is not equal to possible one.
  **/
static bool Gpio_CheckPin(GPIO_TypeDef* pGpio, GpioNumber pin, GpioMode mode)
{
    // Check input pointer to the port object for null value.
    if (!pGpio)
    {
        return false;
    }
    
    uint8_t l_modeBitIndex = 4 * ((uint8_t)pin % 8); ///< Prepare 'Mode' offset.
    bool l_isValid = false; ///< Defines successful type checking.
    // Input/output type checking from GPIO0 to GPIO7.
    if (pin <= kGpio7)
    {
        // Checking input type.
        if (mode == kGpioInput && !((pGpio->CRL >> l_modeBitIndex) & 0x03))
        {
            l_isValid = true;
        }
        // Checking output type.
        else if (mode == kGpioOutput && ((pGpio->CRL >> l_modeBitIndex) & 0x03))
        {
            l_isValid = true;
        }
    }
    // Input/output type checking from GPIO8 to GPIO15.
    else
    {
        // Checking input type.
        if (mode == kGpioInput && !((pGpio->CRH >> l_modeBitIndex) & 0x03))
        {
            l_isValid = true;
        }
        // Checking output type.
        else if (mode == kGpioOutput && ((pGpio->CRH >> l_modeBitIndex) & 0x03))
        {
            l_isValid = true;
        }
    }
    
    return l_isValid;
}

/**
  * @brief  GPIO pin 'Input' mode configuring.
  * @param  port  - GPIO pin port.
  * @param  pin   - number of the GPIO pin.
  * @param  pull  - input pull option.
  * @retval None.
  **/
void Gpio_InitInput(GpioPort port, GpioNumber pin, GpioPull pull)
{
    GPIO_TypeDef* l_pPort = Gpio_GetPort(port); ///< Pointer to GPIO structure
                                                ///  to be used.

    // Enable port clock generating.
    Gpio_EnableClock(port);
    // Setting up GPIO.
    uint8_t l_modeBitIndex = 4 * ((uint8_t)pin % 8);
    uint8_t l_cnfBitIndex  = l_modeBitIndex + 2;
    // Input configuration from GPIO0 to GPIO7.
    if (pin <= kGpio7)
    {
        // Reset GPIO register bits according to pin position.
        l_pPort->CRL &= ~((uint32_t)0x03 << l_modeBitIndex);
        l_pPort->CRL &= ~((uint32_t)0x03 << l_cnfBitIndex);
        // Check if need to configure pull setting.
        if (pull != kGpioNoPull)
        {
            // Configure with the 'Input with pull-up/pull-down' value.
            l_pPort->CRL |= 0x02 << l_cnfBitIndex;
            // Setting up pull feature.
            l_pPort->ODR &= ~((uint32_t)0x01 << (uint8_t)pin);
            l_pPort->ODR |= (0x01 & (uint8_t)pull) << (uint8_t)pin;
        }
        else
        {
            // Configure with the 'Floating input (reset state)' value.
            l_pPort->CRL |= 0x01 << l_cnfBitIndex;
        }
    }
    // Input configuration from GPIO8 to GPIO15.
    else
    {
        // Reset GPIO register bits according to pin position.
        l_pPort->CRH &= ~((uint32_t)0x03 << l_modeBitIndex);
        l_pPort->CRH &= ~((uint32_t)0x03 << l_cnfBitIndex);
        // Check if need to configure pull setting.
        if (pull != kGpioNoPull)
        {
            // Configure with the 'Input with pull-up/pull-down' value.
            l_pPort->CRH |= 0x02 << l_cnfBitIndex;
            // Setting up pull feature.
            l_pPort->ODR &= ~((uint32_t)0x01 << (uint8_t)pin);
            l_pPort->ODR |= (0x01 & (uint8_t)pull) << (uint8_t)pin;
        }
        else
        {
            // Configure with the 'Floating input (reset state)' value.
            l_pPort->CRH |= 0x01 << l_cnfBitIndex;
        }
    }
}

/**
  * @brief  GPIO pin 'Output' mode configuring.
  * @param  port  - GPIO pin port.
  * @param  pin   - number of the GPIO pin.
  * @param  type  - output type.
  * @param  speed - output toggling speed.
  * @param  state - start output signal level (every value above 1 will be
  *                 cut-off to 1).
  * @retval None.
  **/
void Gpio_InitOutput(GpioPort port, GpioNumber pin, GpioOutput type,
                                             GpioSpeed speed, gpio_state state)
{
    GPIO_TypeDef* l_pPort = Gpio_GetPort(port); ///< Pointer to GPIO structure
                                                ///  to be used.

    // Enable port clock generating.
    Gpio_EnableClock(port);
    // Setting up GPIO.
    uint8_t l_modeBitIndex = 4 * ((uint8_t)pin % 8);
    uint8_t l_cnfBitIndex  = l_modeBitIndex + 2;
    // Output configuration from GPIO0 to GPIO7.
    if (pin <= kGpio7)
    {
        // Reset GPIO register bits according to pin position.
        l_pPort->CRL &= ~((uint32_t)0x03 << l_modeBitIndex);
        l_pPort->CRL &= ~((uint32_t)0x03 << l_cnfBitIndex);
        // Setting up pin into 'Output' type using speed value.
        l_pPort->CRL |= (uint8_t)speed << l_modeBitIndex;
        // Setting up output type.
        l_pPort->CRL |= (uint8_t)type << l_cnfBitIndex;
    }
    // Output configuration from GPIO8 to GPIO15.
    else
    {
        // Reset GPIO register bits according to pin position.
        l_pPort->CRH &= ~((uint32_t)0x03 << l_modeBitIndex);
        l_pPort->CRH &= ~((uint32_t)0x03 << l_cnfBitIndex);
        // Setting up pin into 'Output' type using speed value.
        l_pPort->CRH |= (uint8_t)speed << l_modeBitIndex;
        // Setting up output type.
        l_pPort->CRH |= (uint8_t)type << l_cnfBitIndex;
    }
    // Set start pin output signal level.
    gpio_state l_signal = 0x01 & state; ///< Preffered signal level to be
                                        ///  generated out.
    // For setting output to 'High' value should be written into one of the
    // lower 16 bits of the 'BSRR' register. For 'Low' level the '1' should be
    // written into one of the higher 16 bits.
    l_pPort->BSRR = 0x01 << (16 * (1 - l_signal) + (uint8_t)pin); 
}

/**
  * @brief  Input signal level checking.
  * @param  port  - GPIO pin port.
  * @param  pin   - number of the GPIO pin.
  * @retval 1   - 'High' signal level.
  *         0   - 'Low' signal level.
  *         255 - current pin is not in 'Input' mode.
  **/
uint8_t Gpio_InputRead(GpioPort port, GpioNumber pin)
{
    // Check if preffered pin is input.
    GPIO_TypeDef* l_pPort = Gpio_GetPort(port); ///< Pointer to GPIO structure
                                                ///  to be used.    
    if (!Gpio_CheckPin(l_pPort, pin, kGpioInput))
    {
        return 0xFFU;
    }
    // Get input signal level.
    return (0x01 & (l_pPort->IDR >> (uint8_t)pin));
}

/**
  * @brief  Output signal level generating.
  * @param  port  - GPIO pin port.
  * @param  pin   - number of the GPIO pin.
  * @param  state - logic level to be generated (every value above 1 will be
  *                 cut-off to 1).
  * @retval 1 - 'High' signal level.
  *         0 - 'Low' signal level.
  **/
bool Gpio_OutputWrite(GpioPort port, GpioNumber pin, gpio_state state)
{
    // Check if preffered pin is output.
    GPIO_TypeDef* l_pPort = Gpio_GetPort(port); ///< Pointer to GPIO structure
                                                ///  to be used.    
    if (!Gpio_CheckPin(l_pPort, pin, kGpioOutput))
    {
        return false;
    }
    // Set start pin output signal level.
    gpio_state l_signal = 0x01 & state; ///< Preffered signal level to be
                                        ///  generated out.
    // For setting output to 'High' value should be written into one of the
    // lower 16 bits of the 'BSRR' register. For 'Low' level the '1' should be
    // written into one of the higher 16 bits.
    l_pPort->BSRR = 0x01 << (16 * (1 - l_signal) + (uint8_t)pin);
    
    return true;
}

/**
  * @brief  Pin disabling.
  *         Clean up mode and configuration bits for pin should be
  *         disabled.
  *         Does not disable GPIO port clock generating.
  * @param  port  - GPIO pin port.
  * @param  pin   - number of the GPIO pin.
  * @retval None.
  **/
void Gpio_DeInit(GpioPort port, GpioNumber pin)
{
    GPIO_TypeDef* l_pPort = Gpio_GetPort(port); ///< Pointer to GPIO structure
                                                ///  to be used.
    // Reset mode and configuration of the pin.
    uint8_t l_modeBitIndex = 4 * ((uint8_t)pin % 8);
    uint8_t l_cnfBitIndex  = l_modeBitIndex + 2;
    // Output configuration from GPIO0 to GPIO7.
    if (pin <= kGpio7)
    {
        // Reset GPIO register bits according to pin position.
        l_pPort->CRL &= ~((uint32_t)0x03 << l_modeBitIndex);
        l_pPort->CRL &= ~((uint32_t)0x03 << l_cnfBitIndex);
    }
    else
    {
        // Reset GPIO register bits according to pin position.
        l_pPort->CRH &= ~((uint32_t)0x03 << l_modeBitIndex);
        l_pPort->CRH &= ~((uint32_t)0x03 << l_cnfBitIndex);
    }
}