#ifndef __BSP_TIM6_CONTROL_H__
#define __BSP_TIM6_CONTROL_H__

#include "stm32f4xx.h"

/* -----------------------宏定义 -------------------*/
#define TIM6_CONTROL_DEFAULT_PERIOD_MS      1u    // 默认控制周期 1ms
#define TIM6_CONTROL_MIN_PERIOD_MS          1u    // 最小控制周期 1ms
#define TIM6_CONTROL_MAX_PERIOD_MS          1000u // 最大控制周期 1000ms

/* -----------------------函数声明 -------------------*/
void TIM6_Control_Init(uint16_t period_ms);     // 初始化TIM6控制周期定时器
void TIM6_Control_Start(void);                  // 启动TIM6
void TIM6_Control_Stop(void);                   // 停止TIM6
void TIM6_Control_SetPeriod(uint16_t period_ms);  // 设置控制周期 (单位: ms)
uint16_t TIM6_Control_GetPeriod(void);            // 获取当前控制周期 (单位: ms)

#endif /* __BSP_TIM6_CONTROL_H__ */
