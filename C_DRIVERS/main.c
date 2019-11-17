#include "main.h"

#include <string.h>

#include "RCC/CoreRcc.h"
#include "SYSTICK/CoreSysTick.h"
#include "DELAY/CoreDelay.h"
#include "GPIO/PeriphGpio.h"
#include "UART/PeriphUart.h"
#include "RTC/PeriphRtc.h"

int main(void)
{
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
  Uart_Init(kUart1, kUart1TxPA9_RxPA10, kBaud115200, false);
  char l_inputBuffer[384] = {0};
  char l_message[] = "In this article, you'll use Visual Studio to create the "
                     "traditional \"Hello World!\" program.\r\n";
  Uart_Send(kUart1, (uint8_t*)l_message, strlen(l_message));
  // Test RTC.
  Rtc_Init(kHseDiv128, false);
  Rtc_Toggle(true);
  uint8_t l_counter = 0;
  while (++ l_counter < 20) {
      Delay_Wait(1000);
  }
  Rtc_Toggle(false);
  Delay_Wait(2000);
  l_counter = 0;
  Rtc_Toggle(true);
  while (++ l_counter < 5) {
      Delay_Wait(1000);
  }
  Rtc_SetCounter(0);
  l_counter = 0;
  while (++ l_counter < 10) {
      Delay_Wait(1000);
  }
}