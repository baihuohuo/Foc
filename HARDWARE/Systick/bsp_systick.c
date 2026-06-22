#include "bsp_systick.h"

/* 私有类型定义 --------------------------------------------------------------*/
/* 私有宏定义 ----------------------------------------------------------------*/
/* 私有变量 ------------------------------------------------------------------*/
static __IO uint32_t uwTick;

/* 扩展变量 ------------------------------------------------------------------*/
/* 私有函数原形 --------------------------------------------------------------*/
/* 函数体 --------------------------------------------------------------------*/ 


/**
  * 函数功能: 初始化配置系统滴答定时器 SysTick
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
void SysTick_Init(void)
{
	
	  uint32_t prioritygroup = 0x00;
	/* SystemFrequency / 1000    1ms中断一次
	 * SystemFrequency / 100000	 10us中断一次
	 * SystemFrequency / 1000000 1us中断一次
	 */
	SysTick_Config(SystemCoreClock / 1000);
	
	  prioritygroup = NVIC_GetPriorityGrouping();  
  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(prioritygroup, 1, 0));
	
	
}

void BSP_TickInc(void)
{
  uwTick++;
}
uint32_t BSP_GetTick(void)
{
  return uwTick;
}
void BSP_DelayMs(__IO uint32_t Delay)
{
  uint32_t tickstart = 0;
  tickstart = BSP_GetTick();
  while((BSP_GetTick() - tickstart) < Delay)
  {
  }
}
void BSP_SuspendTick(void)
{
  /* Disable SysTick Interrupt */
  CLEAR_BIT(SysTick->CTRL,SysTick_CTRL_TICKINT_Msk);
}
void BSP_ResumeTick(void)
{
  /* Enable SysTick Interrupt */
  SET_BIT(SysTick->CTRL,SysTick_CTRL_TICKINT_Msk);
}

// Systick 回调函数 (由 stm32f4xx_it.c 的 SysTick_Handler 调用)
__weak void BSP_SysTickCallback(void)
{
  /* 默认空实现,用户可在其他文件重新定义 */
}



