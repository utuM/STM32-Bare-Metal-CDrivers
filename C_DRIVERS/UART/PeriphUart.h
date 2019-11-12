/**
  * @file    PeriphUart.h
  * @author  utuM (Kostyantyn Komarov)
  * @version 1.0.0a
  * @date    12.11.19
  * @brief   Universal asynchronous receiver-transmitter (UART) driver.
  *          Provides enabling and disabling UART hardware interface available
  *          by selected ST MCU model, sending and receiving amount of bytes
  *          with all internal setting up, such as clock generating enabling/
  *          disabling, interface pin remapping and so on.
  *          Transmiting is provided in blocking mode, receiving is on
  *          interruption.
  *          To enable one of the available UART interfaces need to put
  *          preffered instance number, mapping and baud rate. No need to enter
  *          number of data bits or stop bits. This driver sets up UART with
  *          this configuration: 8 data bits, 1 stop bit and none parity
  *          control. Recommend to use current driver with the 'PeriphAfio' one
  *          for correct interface pins re-mapping process.
  *          Current version of driver provides sending in blocking mode. Need
  *          to adjust the value of the 'UART_DEFAULT_TIMEOUT' definition if
  *          You need to decrease the maximal waiting time value.
  *          Driver provides data storing using cycle buffer. Every byte that
  *          is came could be processed inside the interrupt function (
  *          'USARTx_IRQHandler', where x - instance number) and, as result,
  *          will be stored inside the handler cycle buffer. So, when You calloc
  *          the reading function it means that driver checks the cycle buffer
  *          for 1 or more bytes to be ready for re-storing. The cycle buffer 
  *          size is limited by the 'UART_RX_BUFFER_SIZE' definition value that
  *          could be changed before every compilation according to needs.
  *          List of functions:
  *          bool Uart_Init(UartInstance, UartMapping, UartBaud) - UART
  *          interface enabling;
  *          bool Uart_Send(UartInstance, uint8_t*, uint16_t) - data bytes
  *          sending;
  *          uint16_t Uart_Read(UartInstance, uint8_t*, uint16_t) - data bytes
  *          reading for the cycle buffer;
  *          bool Uart_DeInit(UartInstance) - UART interface disabling.
  *          Also, in the driver there are a few global variables that should 
  *          be changed. These are:
  *          UART_DEFAULT_TIMEOUT - maximal timeout value in milliseconds
  *          between neighbor byte sendings;
  *          UART_RX_BUFFER_SIZE - size of the handler cycle buffer; should be
  *          defined as 2-byte unsigned value;
  *          DEFAULT_UART1_PRIORITY - UART1 instance interrupt priority;
  *          DEFAULT_UART2_PRIORITY - UART2 instance interrupt priority;
  *          DEFAULT_UART3_PRIORITY - UART3 instance interrupt priority.  
  *          List of supported MCU models:
  *          + STM32F103xB.
  *          Other models will be added in future.
  **/

#ifndef __PERIPHERAL_UART_H
#define __PERIPHERAL_UART_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "stm32f103xb.h"
#include "CoreRcc.h"
#include "CoreSysTick.h"
#include "PeriphAfio.h"

#if defined(STM32F103xB)
#define    UART_INTERFACES_AMOUNT    3
#else
#error "Error: no UART interfaces amount for selected MCU model."
#endif // STM32F103xB

#ifdef __cplusplus
extern "C"
{
#endif 

/**
  * UART instances list.
  **/
typedef enum
{
#if defined(STM32F103xB)
    kUart1  = 0x00U,  ///< UART1 interface.
    kUart2,           ///< UART2 interface.
    kUart3            ///< UART3 interface.
#else
    #error "Error: no UART instances list for selected MCU model."
#endif // STM32F103xB
} UartInstance;

/**
  * UART interfaces mappings list.
  **/
typedef enum
{
#if defined(STM32F103xB)
    // UART1 possible pin configurations.
    kUart1TxPA9_RxPA10  = 0x00U, ///< UART1: 'TX' pin is PA9, 'RX' pin is PA10.
    kUart1TxPB6_RxPB7,           ///< UART1: 'TX' pin is PB6, 'RX' pin is PB7.
    // UART2 possible pin configurations.
    kUart2TxPA2_RxPA3,           ///< UART2: 'TX' pin is PA2, 'RX' pin is PA3.
    kUart2TxPD5_RxPD6,           ///< UART2: 'TX' pin is PD5, 'RX' pin is PD6.
    // UART3 possible pin configurations.
    kUart3TxPB10_RxPB11,         ///< UART3: 'TX' pin is PB10, 'RX' pin is PB11.
    kUart3TxPC10_RxPC11,         ///< UART3: 'TX' pin is PC10, 'RX' pin is PC11.
    kUart3TxPD8_RxPD9            ///< UART3: 'TX' pin is PD8, 'RX' pin is PD9.
#else
    #error "Error: no UART pin configuration list for selected MCU model."
#endif // STM32F103xB
} UartMapping;

/**
  * Possible UART baud rates.
  **/
typedef enum
{
    kBaud1200    = 1200U,    ///< 1.2Kbps rate.
    kBaud2400    = 2400U,    ///< 2.4Kbps rate.
    kBaud4800    = 4800U,    ///< 4.8Kbps rate.
    kBaud9600    = 9600U,    ///< 9.6Kbps rate.
    kBaud19200   = 19200U,   ///< 19.2Kbps rate.
    kBaud38400   = 38400U,   ///< 38.4Kbps rate.
    kBaud57600   = 57600U,   ///< 57.6Kbps rate.
    kBaud115200  = 115200U,  ///< 115.2Kbps rate.
    kBaud230400  = 230400U,  ///< 230.4Kbps rate.
    kBaud460800  = 460800U,  ///< 460.8Kbps rate.
    kBaud921600  = 921600U,  ///< 921.6Kbps rate.
    kBaud2250000 = 2250000U, ///< 2.25Mbps rate.
    kBaud4500000 = 4500000U  ///< 4.5Mbps rate.
} UartBaud;

bool     Uart_Init(UartInstance instance, UartMapping pins, UartBaud rate);
bool     Uart_Send(UartInstance instance, uint8_t* pData, uint16_t size);
uint16_t Uart_Read(UartInstance instance, uint8_t* pData, uint16_t size);
bool     Uart_DeInit(UartInstance instance);

#ifdef __cplusplus
}
#endif

#endif // __PERIPHERAL_UART_H