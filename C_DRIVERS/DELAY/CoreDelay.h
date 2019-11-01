/**
  * @file    CoreDelay.h
  * @author  utuM (Kostyantyn Komarov)
  * @version 1.0.0a
  * @date    31.10.19
  * @brief   System time delayer driver.
  *          Provides system delays using system timer ticks and input amount
  *          of units to be waited.
  *          Driver takes current ticks amount from the system timer, save the
  *          summary of this value and input units amount and wait until
  *          current system tick value is equal to the summary.
  *          Delay operation is hardly bound with the system timer settings, 
  *          especially with tick units.
  *          In this driver can be found only one function:
  *          void Delay_Wait(uint32_t) - provides time delay.
  **/

#ifndef __CORE_DELAY_H
#define __CORE_DELAY_H

#include <stdint.h>
#include "CoreSysTick.h"

#ifdef __cplusplus
extern "C"
{
#endif

void Delay_Init(uint16_t ticks);
void Delay_Wait(uint32_t units);
    
#ifdef __cplusplus
}
#endif

#endif // __CORE_DELAY_H