#include "main.h"

#include "CoreRcc.h"
#include "CoreSysTick.h"
#include "CoreDelay.h"
#include "PeriphGpio.h"

/**
  * @brief  The application entry point.
  * @retval int
  */
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
  Gpio_InitOutput(kGpioB, kGpio11, kGpioPushPull, kGpioOutputMid, 1);
  printf("System tick length is %u ms.\r\n", SysTick_GetTickLength());
  
  uint32_t l_counter = 0;
  while (1)
  {
    Gpio_OutputWrite(kGpioB, kGpio11, l_counter % 2);
	++ l_counter;
    printf("Input states: %u %u %u\r\n", Gpio_InputRead(kGpioA, kGpio9),
                                         Gpio_InputRead(kGpioA, kGpio10),
                                         Gpio_InputRead(kGpioA, kGpio11));
    
	// Test delay.
    Delay_Wait(1000);
  }
}