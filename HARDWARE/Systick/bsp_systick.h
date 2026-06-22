#ifndef __BSP_SYSTICK_H
#define __BSP_SYSTICK_H

#include "stm32f4xx.h"

void SysTick_Init(void);
void BSP_TickInc(void);
uint32_t BSP_GetTick(void);
void BSP_DelayMs(__IO uint32_t Delay);
void BSP_SuspendTick(void);
void BSP_ResumeTick(void);
void BSP_SysTickCallback(void);

	 				    
#endif

