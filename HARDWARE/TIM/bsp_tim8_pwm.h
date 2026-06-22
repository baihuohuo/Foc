#ifndef __BSP_TIM8_PWM_H
#define __BSP_TIM8_PWM_H

#include "stm32f4xx.h"

/* -----------------------类型定义 -------------------*/
typedef struct
{
  uint16_t DutyA;  // A相占空比计数值, 对应 TIM8->CCR1
  uint16_t DutyB;  // B相占空比计数值, 对应 TIM8->CCR2
  uint16_t DutyC;  // C相占空比计数值, 对应 TIM8->CCR3
}TIM8_PWM_DutyTypeDef;

/* -----------------------宏定义 -------------------*/
#define TIM8_PWM_FREQUENCY_HZ         20000u      /* PWM频率 20kHz */
#define TIM8_PWM_DEAD_TIME            84u         /* 死区时间 84ns */
#define TIM8_PWM_DUTY_MIN             0u          /* 最小占空比 */

/* -----------------------函数声明 -------------------*/
// TIM8 PWM相关函数
void TIM8_PWM_GPIO_Init(void);                              // GPIO初始化
void TIM8_PWM_Init(void);                                  // PWM初始化
void TIM8_PWM_Start(void);                                 // 启动PWM
void TIM8_PWM_Stop(void);                                  // 停止PWM
void TIM8_PWM_ModuleInit(void);                            // TIM8 PWM模块一键初始化
void TIM8_PWM_SetDuty(uint16_t duty_a, uint16_t duty_b, uint16_t duty_c);  // 设置三相占空比
void TIM8_PWM_SetDutyStruct(TIM8_PWM_DutyTypeDef duty);   // 结构体方式设置占空比
uint16_t TIM8_PWM_GetPeriod(void);                         // 获取PWM周期

#endif /* __BSP_TIM8_PWM_H */
