/**
  * @brief  File for relay_proc.c module
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _RELAY_PROC_H_
#define _RELAY_PROC_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
/* Defines -------------------------------------------------------------------*/
#define RELAY_DEF1      (1 << 0)
#define RELAY_DEF2      (1 << 1)
#define RELAY_DEF3      (1 << 2)
#define RELAY_DEF4      (1 << 3)
/* Exported constants --------------------------------------------------------*/

/* Prototypes ----------------------------------------------------------------*/
void RELAY_DirectSwitch(uint8_t state);
void RELAY_Proc(void);
#endif