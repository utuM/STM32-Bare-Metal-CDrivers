/**
  * @file    PeriphUart.c
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

#include "PeriphUart.h"

#define    UART_DEFAULT_TIMEOUT      1000U ///< Default sending timeout,
                                           ///  in milliseconds.
#define    UART_RX_BUFFER_SIZE       256U  ///< Default receive buffer size.

#define    DEFAULT_UART1_PRIORITY    0U    ///< UART1 interrupt priority level.
#define    DEFAULT_UART2_PRIORITY    0U    ///< UART2 interrupt priority level.
#define    DEFAULT_UART3_PRIORITY    0U    ///< UART3 interrupt priority level.

/**
  * UART interface handler.
  **/
typedef struct
{
    bool m_isInit;                     ///< UART ready flag.
    bool m_isLocked;                   ///< UART instance locking flag.
    UartMapping m_mapping;             ///< Current UART interface mapping.
    uint8_t m_rx[UART_RX_BUFFER_SIZE]; ///< Receive buffer.
    uint16_t m_rxTail;                 ///< The first byte index in the cycle
                                       ///  buffer.
    uint16_t m_rxHead;                 ///< The last byte index in the cycle
                                       ///  buffer. 
    bool m_reserved;                   ///< Reserved for the future update.
} UartHandler;

static UartHandler s_uart[UART_INTERFACES_AMOUNT] = ///< UART handlers.
{
    {
        .m_isInit       = false,
        .m_isLocked     = false,
        .m_mapping      = kUart1TxPA9_RxPA10,
        .m_rx           = {0},
        .m_rxTail       = 0,
        .m_rxHead       = 0,
        .m_reserved     = false
    },
    {
        .m_isInit       = false,
        .m_isLocked     = false,
        .m_mapping      = kUart1TxPA9_RxPA10,
        .m_rx           = {0},
        .m_rxTail       = 0,
        .m_rxHead       = 0,
        .m_reserved     = false
    },
    {
        .m_isInit       = false,
        .m_isLocked     = false,
        .m_mapping      = kUart1TxPA9_RxPA10,
        .m_rx           = {0},
        .m_rxTail       = 0,
        .m_rxHead       = 0,
        .m_reserved     = false
    }
};

static USART_TypeDef* Uart_GetPeripheral(UartInstance instance);
static void Uart_EnableClock(UartInstance instance);
static void Uart_DisableClock(UartInstance instance);
static bool Uart_CheckRxBytes(UartInstance instance);

static void USART1_IRQHandler(void);
static void USART2_IRQHandler(void);
static void USART3_IRQHandler(void);

/**
  * @brief  Getting UART interface structure object using instance number.
  * @param  instance - number of UART instance.
  * @retval Pointer to the UART interface structure.
  **/
static USART_TypeDef* Uart_GetPeripheral(UartInstance instance)
{
    // Return pointer to UART peripheral structure.
    switch (instance)
    {
        case kUart1:
            return USART1;

        case kUart2:
            return USART2;
        
        case kUart3:
            return USART3;
    }

    return 0; // To prevent compiler warning.
}

/**
  * @brief  UART clock generating enabling.
  * @param  instance - number of UART instance.
  * @retval None.
  **/
static void Uart_EnableClock(UartInstance instance)
{
    // Enable clock generating according to input instance.
    switch (instance)
    {
        case kUart1:
            RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
            break;

        case kUart2:
            RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
            break;

        case kUart3:
            RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
            break;
    }
}

/**
  * @brief  UART clock generating disabling.
  * @param  instance - number of UART instance.
  * @retval None.
  **/
static void Uart_DisableClock(UartInstance instance)
{
    // Enable clock generating according to input instance.
    switch (instance)
    {
        case kUart1:
            RCC->APB2ENR &= ~(RCC_APB2ENR_USART1EN);
            break;

        case kUart2:
            RCC->APB1ENR &= ~(RCC_APB1ENR_USART2EN);
            break;

        case kUart3:
            RCC->APB1ENR &= ~(RCC_APB1ENR_USART3EN);
            break;
    }    
}

/**
  * @brief  Checking for available bytes in the receive buffer.
  * @param  instance - number of UART instance.
  * @retval true  - at least 1 byte in the receive buffer is ready to be read;
  *         false - no available bytes. 
  **/
static bool Uart_CheckRxBytes(UartInstance instance)
{
    if (!s_uart[instance].m_isInit)
    {
        return 0U;
    }

    // Check amount of bytes ready to be read.
    return (s_uart[instance].m_rxTail != s_uart[instance].m_rxHead);
}

/**
  * @brief  UART1 interface interrupt processing.
  * @param  None.
  * @retval None.
  **/
static void USART1_IRQHandler(void)
{
    s_uart[0].m_isLocked = true;
    USART_TypeDef* l_pUart = Uart_GetPeripheral(kUart1);
    // Check if receiver is enabled, receiving interruption is enabled and data
    // register is empty.
    if (USART1->SR & USART_SR_RXNE || USART1->SR & USART_SR_ORE)
    {
        USART1->SR &= ~(USART_SR_RXNE);
        // Put byte in the buffer.
        s_uart[0].m_rx[s_uart[0].m_rxHead ++] = 0xFF & USART1->DR;
        s_uart[0].m_rxHead %= UART_RX_BUFFER_SIZE;
    }
    // Check if transmitter is enabled, transmission interruption is enabled and
    // data register is empty.
    else if ((USART1->CR1 & USART_CR1_TE) && (USART1->CR1 & USART_CR1_TCIE) &&
        (USART1->SR & USART_SR_TC))
    {
        USART1->SR &= ~(USART_SR_TC);
    }
    s_uart[0].m_isLocked = false;
}

/**
  * @brief  UART2 interface interrupt processing.
  * @param  None.
  * @retval None.
  **/
static void USART2_IRQHandler(void)
{
    s_uart[1].m_isLocked = true;
    // Check if receiver is enabled, receiving interruption is enabled and data
    // register is empty.
    if ((USART2->CR1 & USART_CR1_RE) && (USART2->CR1 & USART_CR1_RXNEIE) &&
        (USART2->SR & USART_SR_RXNE))
    {
        USART2->SR &= ~(USART_SR_RXNE);
        // Put byte in the buffer.
        s_uart[1].m_rx[s_uart[1].m_rxHead ++] = 0xFF & USART1->DR;
        s_uart[1].m_rxHead %= UART_RX_BUFFER_SIZE;
    }
    // Check if transmitter is enabled, transmission interruption is enabled and
    // data register is empty.
    if ((USART2->CR1 & USART_CR1_TE) && (USART2->CR1 & USART_CR1_TCIE) &&
        (USART2->SR & USART_SR_TC))
    {
        USART2->SR &= ~(USART_SR_TC);
    }
    s_uart[1].m_isLocked = false;
}

/**
  * @brief  UART3 interface interrupt processing.
  * @param  None.
  * @retval None.
  **/
static void USART3_IRQHandler(void)
{
    s_uart[2].m_isLocked = true;
    // Check if receiver is enabled, receiving interruption is enabled and data
    // register is empty.
    if ((USART3->CR1 & USART_CR1_RE) && (USART3->CR1 & USART_CR1_RXNEIE) &&
        (USART3->SR & USART_SR_RXNE))
    {
        USART3->SR &= ~(USART_SR_RXNE);
        // Put byte in the buffer.
        s_uart[2].m_rx[s_uart[2].m_rxHead ++] = 0xFF & USART1->DR;
        s_uart[2].m_rxHead %= UART_RX_BUFFER_SIZE;
    }
    // Check if transmitter is enabled, transmission interruption is enabled and
    // data register is empty.
    if ((USART3->CR1 & USART_CR1_TE) && (USART3->CR1 & USART_CR1_TCIE) &&
        (USART3->SR & USART_SR_TC))
    {
        USART3->SR &= ~(USART_SR_TC);
    }
    s_uart[2].m_isLocked = false;    
}

/**
  * @brief  UART interface enabling.
  * @param  instance - number of UART instance.
  * @param  pins     - preffered UART mapping.
  * @param  rate     - baud rate for current UART instance.
  * @retval true  - UART handler is enabled successful;
  *         false - UART handler is already initialized or selected mapping is
  *                 not valid for current instance number.
  **/
bool Uart_Init(UartInstance instance, UartMapping pins, UartBaud rate)
{
    // Check if UART interface is ready.
    if (s_uart[instance].m_isInit)
    {
        return false;
    }
    // Check if the UART mapping is valid for the selected UART instance.
    switch (instance)
    {
        case kUart1:
            if (pins >= kUart2TxPA2_RxPA3)
            {
                return false;
            }                
            break;
            
        case kUart2:
            if (pins != kUart2TxPA2_RxPA3 && pins != kUart2TxPD5_RxPD6)
            {
                return false;
            }                
            break;
            
        case kUart3:
            if (pins <= kUart2TxPA2_RxPA3)
            {
                return false;
            }                
            break;
    }

    USART_TypeDef* l_pUart; ///< Pointer to the UART structure.
    l_pUart = Uart_GetPeripheral(instance);
    // Disable UART.
    l_pUart->CR1 &= ~(USART_CR1_UE);
    // Clock generating enabling.
    Uart_EnableClock(instance);
    // Enable pins re-mapping.
    switch (pins)
    {
        case kUart1TxPA9_RxPA10:
            AFIO->MAPR &= ~(AFIO_MAPR_USART1_REMAP);
            Afio_Init(kAfioA, kAfio9,  kAfioPushPull);
            Afio_Init(kAfioA, kAfio10, kAfioInputNoPull);
            break;
    
        case kUart1TxPB6_RxPB7:
            AFIO->MAPR |= AFIO_MAPR_USART1_REMAP;
            Afio_Init(kAfioB, kAfio6, kAfioPushPull);
            Afio_Init(kAfioB, kAfio7, kAfioInputNoPull);
            break;

        case kUart2TxPA2_RxPA3:
            AFIO->MAPR &= ~(AFIO_MAPR_USART2_REMAP);
            Afio_Init(kAfioA, kAfio2, kAfioPushPull);
            Afio_Init(kAfioA, kAfio3, kAfioInputNoPull);
            break;

        case kUart2TxPD5_RxPD6:
            AFIO->MAPR |= AFIO_MAPR_USART2_REMAP;
            Afio_Init(kAfioD, kAfio5, kAfioPushPull);
            Afio_Init(kAfioD, kAfio6, kAfioInputNoPull);
            break;

        case kUart3TxPB10_RxPB11:
            AFIO->MAPR &= ~(AFIO_MAPR_USART3_REMAP);
            Afio_Init(kAfioB, kAfio10, kAfioPushPull);
            Afio_Init(kAfioB, kAfio11, kAfioInputNoPull);
            break;

        case kUart3TxPC10_RxPC11:
            AFIO->MAPR |= AFIO_MAPR_USART3_REMAP_0;
            Afio_Init(kAfioC, kAfio10, kAfioPushPull);
            Afio_Init(kAfioC, kAfio11, kAfioInputNoPull);
            break;

        case kUart3TxPD8_RxPD9:
            AFIO->MAPR |= AFIO_MAPR_USART3_REMAP;
            Afio_Init(kAfioD, kAfio8, kAfioPushPull);
            Afio_Init(kAfioD, kAfio9, kAfioInputNoPull);
            break;
    }
    // Save current interface mapping.
    s_uart[instance].m_mapping = pins;
    // Calculate baud rate value and setting up.
    float l_baudDivider = 0.0f; ///< Baud divider to be written into register.
    uint8_t l_busPrescaler = 0; ///< Bus prescaler value.
    if (instance == kUart1)
    {
        l_busPrescaler = Rcc_GetApb2Divider();
    }
    else
    {
        l_busPrescaler = Rcc_GetApb1Divider();
    }
    l_baudDivider = Rcc_GetSystemClock() / (float)l_busPrescaler / rate / 16.0f;
    uint32_t l_baud = (uint32_t)(l_baudDivider * 10);
    uint32_t l_baudCode = 0x0000FFFF & (((l_baud / 10) << 4) | (l_baud % 10));
    l_pUart->BRR = 0x0000FFFF & l_baudCode;
    // Select mode: 1 start bit, 8 data bits, n stop bits, disable parity
    // control.
    l_pUart->CR1 &= ~(USART_CR1_M | USART_CR1_PCE);
    // Select 1 stop bit.
    l_pUart->CR2 &= ~(USART_CR2_STOP);
    // Disable all useless settings.
    l_pUart->CR1 &= ~(USART_CR1_SBK | USART_CR1_PEIE | USART_CR1_WAKE);
    l_pUart->CR2 &= ~(USART_CR2_LINEN | USART_CR2_CLKEN | USART_CR2_LBDIE);
    l_pUart->CR3 &= ~(USART_CR3_SCEN | USART_CR3_HDSEL | USART_CR3_IREN);
    // Enable transmitter and receiver.
    l_pUart->CR1 |= USART_CR1_TE | USART_CR1_RE;
    // Enable receiving and transmiting interruptions.
    l_pUart->CR1 |= USART_CR1_TCIE | USART_CR1_RXNEIE;
    // Enable error interruption.
    //l_pUart->CR3 |= USART_CR3_EIE;
    // Enable the UART interface.
    l_pUart->CR1 |= USART_CR1_UE;
    // Clear all status flag before proceed.
    l_pUart->SR &= ~(USART_SR_RXNE | USART_SR_TC | USART_SR_LBD | USART_SR_CTS);
    // Enable interrupts with preferred priority level.
    switch (instance)
    {
        case kUart1:
            NVIC_SetPriority(USART1_IRQn, DEFAULT_UART1_PRIORITY);
            NVIC_EnableIRQ(USART1_IRQn);
            break;

        case kUart2:
            NVIC_SetPriority(USART2_IRQn, DEFAULT_UART2_PRIORITY);
            NVIC_EnableIRQ(USART2_IRQn);
            break;

        case kUart3:
            NVIC_SetPriority(USART3_IRQn, DEFAULT_UART3_PRIORITY);
            NVIC_EnableIRQ(USART3_IRQn);
            break;
    }
    s_uart[instance].m_isInit = true;

    return true;
}

/**
  * @brief  Bytes sending via UART.
  * @param  instance - number of UART instance to be used.
  * @param  pData    - pointer to the buffer filled with bytes to be sent.
  * @param  size     - size of input buffer.
  * @retval true  - buffer is sent successful;
  *         false - UART handler is not ready, pointer is null, size of buffer
  *                 is 0, UART handler is locked for current sending operation
  *                 or not all bytes were sent.
  **/
bool Uart_Send(UartInstance instance, uint8_t* pData, uint16_t size)
{
    // Check if UART interface is ready for sending.
    if (!s_uart[instance].m_isInit || !pData || !size ||
                                                   s_uart[instance].m_isLocked)
    {
        return false;
    }
        
    // Lock the UART handler.
    bool l_isSuccessful = false;
    s_uart[instance].m_isLocked = true;
    USART_TypeDef* l_pUart; ///< Pointer to USART structure.
    l_pUart = Uart_GetPeripheral(instance);
    // Clear all status flag before proceed.
    l_pUart->SR &= ~(USART_SR_RXNE | USART_SR_TC | USART_SR_LBD |
                                                                 USART_SR_CTS);
    // Check current transmission status and send byte.
    uint64_t l_endTick = UART_DEFAULT_TIMEOUT + SysTick_GetTick();
    uint16_t l_index = 0;
    do
    {
        if (l_pUart->SR & USART_SR_TXE)
        {
            l_pUart->DR = 0xFF & pData[l_index ++];
            if (l_index > size)
            {
                break;
            }
            l_endTick = UART_DEFAULT_TIMEOUT + SysTick_GetTick();
        }
    } while (l_endTick >= SysTick_GetTick());
    // Check if all bytes were sent.
    if (l_index > size)
    {
        l_isSuccessful = true;
    }
    s_uart[instance].m_isLocked = false;
    
    return l_isSuccessful;
}

/**
  * @brief  Bytes reading from the UART handler cycle buffer.
  * @param  instance - number of UART instance to be used.
  * @param  pData    - pointer to the buffer need to be filled by read bytes.
  * @param  size     - size of input buffer.
  * @retval true  - buffer is filled successful;
  *         false - UART handler is not ready, pointer is null, size of buffer
  *                 is 0, UART handler is locked for current sending operation
  *                 or there is no available bytes to be read from the UART
  *                 cycle buffer.
  **/
uint16_t Uart_Read(UartInstance instance, uint8_t* pData, uint16_t size)
{
    // Check current instance, input buffer pointer and size.
    if (!s_uart[instance].m_isInit || !pData || !size ||
                                                   s_uart[instance].m_isLocked)
    {
        return 0U;
    }

    // Check if some bytes ready to be read.
    if (!Uart_CheckRxBytes(instance))
    {
        return 0U;
    }
    uint16_t l_bytesToBeRead = 0U; ///< Available bytes for reading limited by
                                   ///  input size of buffer or by amount of 
                                   ///  the available bytes.
    s_uart[instance].m_isLocked = true;
    // Check amount of bytes ready to be read.
    if (s_uart[instance].m_rxHead < s_uart[instance].m_rxTail)
    {
        l_bytesToBeRead = UART_RX_BUFFER_SIZE + s_uart[instance].m_rxHead -
                                                      s_uart[instance].m_rxTail;
    }
    else
    {
        l_bytesToBeRead = s_uart[instance].m_rxHead - s_uart[instance].m_rxTail;
    }
    // Limit amount of available bytes according to input buffer size.
    if (l_bytesToBeRead > size)
    {
        l_bytesToBeRead = size;
    }
    // Put received bytes into the input buffer by its pointer.
    do
    {
        *(pData ++) = s_uart[instance].m_rx[s_uart[instance].m_rxTail ++];
        s_uart[instance].m_rxTail %= UART_RX_BUFFER_SIZE;
    } while (s_uart[instance].m_rxTail != s_uart[instance].m_rxHead);
    s_uart[instance].m_isLocked = false;

    return l_bytesToBeRead;
}

/**
  * @brief  UART interface disabling.
  * @param  instance - number of UART instance.
  * @retval true  - UART handler is disabled successful;
  *         false - UART handler is already disabled.
  **/
bool Uart_DeInit(UartInstance instance)
{
    // Check current instance, input buffer pointer and size.
    if (!s_uart[instance].m_isInit)
    {
        return false;
    }
   
    // Disable UART interface.
    switch (instance)
    {
        case kUart1:
            NVIC_DisableIRQ(USART1_IRQn);
            USART1->CR1 &= ~(USART_CR1_UE);
            switch (s_uart[instance].m_mapping)
            {
                case kUart1TxPA9_RxPA10:
                    Afio_DeInit(kAfioA, kAfio9);
                    Afio_DeInit(kAfioA, kAfio10);
                    break;
    
                case kUart1TxPB6_RxPB7:
                    Afio_DeInit(kAfioB, kAfio6);
                    Afio_DeInit(kAfioB, kAfio7);
                    break;
            }
            break;

        case kUart2:
            NVIC_DisableIRQ(USART2_IRQn);
            USART2->CR1 &= ~(USART_CR1_UE);
            switch (s_uart[instance].m_mapping)
            {
                case kUart2TxPA2_RxPA3:
                    Afio_DeInit(kAfioA, kAfio2);
                    Afio_DeInit(kAfioA, kAfio3);
                    break;
    
                case kUart2TxPD5_RxPD6:
                    Afio_DeInit(kAfioD, kAfio5);
                    Afio_DeInit(kAfioD, kAfio6);
                    break;
            }
            break;

        case kUart3:
            NVIC_DisableIRQ(USART3_IRQn);
            USART3->CR1 &= ~(USART_CR1_UE);
            switch (s_uart[instance].m_mapping)
            {
                case kUart3TxPB10_RxPB11:
                    Afio_DeInit(kAfioB, kAfio10);
                    Afio_DeInit(kAfioB, kAfio11);
                    break;
    
                case kUart3TxPC10_RxPC11:
                    Afio_DeInit(kAfioC, kAfio10);
                    Afio_DeInit(kAfioC, kAfio11);
                    break;

                case kUart3TxPD8_RxPD9:
                    Afio_DeInit(kAfioD, kAfio8);
                    Afio_DeInit(kAfioD, kAfio9);
                    break;
            }
            break;
    }
    // Disable UART clocking.
    Uart_DisableClock(instance);
    // Clean up instance handler before leave.
    memset(s_uart[instance].m_rx, 0x00, UART_RX_BUFFER_SIZE);
    s_uart[instance].m_rxHead = 0;
    s_uart[instance].m_rxTail = 0;
    s_uart[instance].m_isInit = false;

    return true;
}