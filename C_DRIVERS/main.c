
#include "main.h"

#include "RCC/CoreRcc.h"
#include "SYSTICK/CoreSysTick.h"

int main(void)
{
  Rcc_InitCore(kClock24MHz);
  SysTick_Init(kTickMillisecs, 1);

  while (1);
}