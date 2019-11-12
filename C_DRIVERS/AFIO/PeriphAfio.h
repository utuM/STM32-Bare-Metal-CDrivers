/**
  * @file    PeriphAfio.h
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

#ifndef __PERIPHERAL_AFIO_H
#define __PERIPHERAL_AFIO_H

#include <stdbool.h>
#include <stdint.h>

#include "stm32f103xb.h"

/**
  * List of possible AFIO ports.
  **/
typedef enum
{
#if defined(STM32F103xB)
    kAfioA = 0x00U, ///< AFIO port A.
    kAfioB,         ///< AFIO port B.
    kAfioC,         ///< AFIO port C.
    kAfioD          ///< AFIO port D.
#else
    #error "Error: no AFIO port list for selected MCU model."
#endif // STM32F103xB
} AfioPort;

/**
  * List of possible AFIO numbers.
  **/
typedef enum
{
    kAfio0   = 0x00U, ///< AFIO number 0.
    kAfio1,           ///< AFIO number 1.
    kAfio2,           ///< AFIO number 2.
    kAfio3,           ///< AFIO number 3.
    kAfio4,           ///< AFIO number 4.
    kAfio5,           ///< AFIO number 5.
    kAfio6,           ///< AFIO number 6.
    kAfio7,           ///< AFIO number 7.
    kAfio8,           ///< AFIO number 8.
    kAfio9,           ///< AFIO number 9.
    kAfio10,          ///< AFIO number 10.
    kAfio11,          ///< AFIO number 11.
    kAfio12,          ///< AFIO number 12.
    kAfio13,          ///< AFIO number 13.
    kAfio14,          ///< AFIO number 14.
    kAfio15           ///< AFIO number 15.
} AfioNumber;

/**
  * Alternate function output type.
  **/
typedef enum
{
    kAfioInputPullDown = 0x00U, ///< Alternate function input, pulled-down. 
    kAfioInputPullUp,           ///< Alternate function input, pulled-up. 
    kAfioPushPull,              ///< Alternate function push-pull.
    kAfioOpenDrain,             ///< Alternate function open-drain.
    kAfioInputNoPull            ///< Alternate function floating input.
} AfioType;

#ifdef __cplusplus
extern "C"
{
#endif

void Afio_Init(AfioPort port, AfioNumber pin, AfioType type);
void Afio_DeInit(AfioPort port, AfioNumber pin);

#ifdef __cplusplus
}
#endif

#endif // __PERIPHERAL_AFIO_H