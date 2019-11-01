/**
  * @file    PeriphGpio.h
  * @author  utuM (Kostyantyn Komarov)
  * @version 1.0.0a
  * @date    31.10.19
  * @brief   General purpose input/output driver.
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
  *          List of function:
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

#ifndef __PERIPHERAL_GPIO_H
#define __PERIPHERAL_GPIO_H

#include <stdbool.h>
#include <stdint.h>

#include "stm32f103xb.h"

typedef unsigned char gpio_state; ///< Only for GPIO output level toggling.

/**
  * List of possible GPIO ports.
  **/
typedef enum
{
#if defined(STM32F103xB)
    kGpioA = 0x00U, ///< GPIO port A.
    kGpioB,         ///< GPIO port B.
    kGpioC,         ///< GPIO port C.
    kGpioD          ///< GPIO port D.
#else
    #error "Error: no GPIO port list for selected MCU model."
#endif // STM32F103xB
} GpioPort;

/**
  * List of possible GPIO numbers.
  **/
typedef enum
{
    kGpio0   = 0x00U, ///< GPIO number 0.
    kGpio1,           ///< GPIO number 1.
    kGpio2,           ///< GPIO number 2.
    kGpio3,           ///< GPIO number 3.
    kGpio4,           ///< GPIO number 4.
    kGpio5,           ///< GPIO number 5.
    kGpio6,           ///< GPIO number 6.
    kGpio7,           ///< GPIO number 7.
    kGpio8,           ///< GPIO number 8.
    kGpio9,           ///< GPIO number 9.
    kGpio10,          ///< GPIO number 10.
    kGpio11,          ///< GPIO number 11.
    kGpio12,          ///< GPIO number 12.
    kGpio13,          ///< GPIO number 13.
    kGpio14,          ///< GPIO number 14.
    kGpio15           ///< GPIO number 15.
} GpioNumber;

/**
  * GPIO possible modes.
  **/
typedef enum
{
    kGpioInput  = 0x00, ///< Pin input type.
    kGpioOutput         ///< Pin output type.
} GpioMode;

/**
  * GPIO input pull type.
  **/
typedef enum
{
    kGpioPullDown = 0x00U, ///< Input with pull-down.
    kGpioPullUp,           ///< Input with pull-up.
    kGpioNoPull            ///< No pull-up/pull-down input (floating state).
} GpioPull;

/**
  * Output type.
  **/
typedef enum
{
    kGpioPushPull  = 0x00U, ///< Push-pull output.
    kGpioOpenDrain          ///< Open-drain output.
} GpioOutput;

/**
  * Output speed.
  **/
typedef enum
{
    kGpioOutputMid  = 0x01U, ///< Output speed 10MHz.
    kGpioOutputLow,          ///< Output speed 2MHz.
    kGpioOutputHigh          ///< Output speed 50MHz.
} GpioSpeed;

#ifdef __cplusplus
extern "C"
{
#endif

void    Gpio_InitInput(GpioPort port, GpioNumber pin, GpioPull pull);
void    Gpio_InitOutput(GpioPort port, GpioNumber pin, GpioOutput type,
                                             GpioSpeed speed, gpio_state state);
uint8_t Gpio_InputRead(GpioPort port, GpioNumber pin);
bool    Gpio_OutputWrite(GpioPort port, GpioNumber pin, gpio_state state);
void    Gpio_DeInit(GpioPort port, GpioNumber pin);

#ifdef __cplusplus
}
#endif

#endif // __PERIPHERAL_GPIO_H