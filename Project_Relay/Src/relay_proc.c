/**
  * This file contains functions for relay direct control
  */
/* Includes ------------------------------------------------------------------*/
#include "relay_proc.h"
#include "gpio.h"
/* Private variables ---------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/
/**
  * @brief Main relay control function
  */
void RELAY_Proc(void)
{
  uint8_t state = 0x01;
  
  while (state <= 0x08)
  {
    RELAY_DirectSwitch(state);
    HAL_Delay(1000);
    state *= 2;
  }
}



/**
  * @brief Function for direct switch relay state
  * @param 8 bit value for 4 bit relay status. 1 - on, 0 - off
  */
void RELAY_DirectSwitch(uint8_t state)
{
  if (state & RELAY_DEF1)
  {
    RELAY_IN1_ON;
  }
  else
  {
    RELAY_IN1_OFF;
  }
  if (state & RELAY_DEF2)
  {
    RELAY_IN2_ON;
  }
  else
  {
    RELAY_IN2_OFF;
  }
  if (state & RELAY_DEF3)
  {
    RELAY_IN3_ON;
  }
  else
  {
    RELAY_IN3_OFF;
  }
  if (state & RELAY_DEF4)
  {
    RELAY_IN4_ON;
  }
  else
  {
    RELAY_IN4_OFF;
  }
}