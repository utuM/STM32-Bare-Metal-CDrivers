/**
  * @file    CoreDelay.c
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

#include "CoreDelay.h"

/**
  * @brief  Process time delay.
  *         Holds microcontroller in cycle until current tick value is equal
  *         to some end value which calculated before cycle is started as a
  *         sum of the ticks counter on that moment and amount of units.
  *         Notice, that unit is equal to 1 tick of system timer and, according
  *         to this, the delay time could be different. For example, if system
  *         timer tick length is setting up with 5 milliseconds, then 1 unit is
  *         5 milliseconds. In this case, when You put 'units' defined as a
  *         value '10', then time delay will be 5 * 10 = 50 milliseconds.  
  * @param  units - amount of ticks to be held.
  * @retval None.
  **/
void Delay_Wait(uint32_t units)
{
    // Get current tick value.
    uint64_t l_tick = SysTick_GetTick();
    // Save the end tick value using input delay one.     
    uint64_t l_endTick = l_tick + units;
    // Wait until current tick value is equal to the end one.
    while (SysTick_GetTick() <= l_endTick);
}