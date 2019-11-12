#include "RCC/CoreRcc.h"
#include "SYSTICK/CoreSysTick.h"
#include "DELAY/CoreDelay.h"
#include "GPIO/PeriphGpio.h"
#include "UART/PeriphUart.h"

int main(void)
{
  /* USER CODE BEGIN 1 */
  // Test RCC.
  Rcc_InitCore(kClock48MHz);
  // Test SysTick.
  SysTick_Init(kTickMillisecs, 1);
  // Test GPIOs.
  Gpio_InitInput(kGpioA, kGpio9, kGpioPullDown);
  Gpio_InitInput(kGpioA, kGpio10, kGpioPullUp);
  Gpio_InitInput(kGpioA, kGpio11, kGpioPullDown);
  Gpio_InitOutput(kGpioB, kGpio6, kGpioPushPull, kGpioOutputMid, 0);
  Gpio_InitOutput(kGpioB, kGpio11, kGpioPushPull, kGpioOutputMid, 1);
  Gpio_InitOutput(kGpioB, kGpio10, kGpioPushPull, kGpioOutputHigh, 0);
  // Test UARTs.
  Uart_Init(kUart1, kUart1TxPA9_RxPA10, kBaud115200);
  char l_inputBuffer[384] = {0};
  char l_message[] = "In this article, you'll use Visual Studio to create the "
                     "traditional \"Hello World!\" program.\r\n";
  uint16_t l_read = 0;
  Uart_Send(kUart1, (uint8_t*)l_message, strlen(l_message));
  while (1) {
    l_read = Uart_Read(kUart1, (uint8_t*)l_inputBuffer, 384);
    l_inputBuffer[l_read] = 0x00;
    if (l_read) {
        printf("%s", l_inputBuffer);
        l_read = 0;
    }
    Delay_Wait(20);
  }
}