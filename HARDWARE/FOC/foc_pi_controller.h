#ifndef __FOC_PI_CONTROLLER_H__
#define __FOC_PI_CONTROLLER_H__

#include "stm32f4xx.h"

/* -----------------------类型定义 -------------------*/
/**
 * @brief 增量式PI控制器
 *
 * 增量式PI公式:
 *   error_now  = target - feedback
 *   delta_out  = Kp * (error_now - error_last) + Ki * error_now * dt
 *   output_now = output_last + delta_out
 */
typedef struct
{
    float Kp;             // 比例系数
    float Ki;             // 积分系数
    float ControlPeriod;  // 控制周期, 单位: s

    float OutputMin;      // 输出下限
    float OutputMax;      // 输出上限

    float ErrorLast;      // 上一次误差
    float OutputLast;     // 上一次输出值
} FOC_PIControllerTypeDef;

/* -----------------------函数声明 -------------------*/
void FOC_PIController_Init(FOC_PIControllerTypeDef *pi,       // 初始化PI控制器
                           float kp,                          // 比例系数
                           float ki,                          // 积分系数
                           float control_period,              // 控制周期, 单位: s
                           float output_min,                  // 输出下限
                           float output_max);                 // 输出上限

void FOC_PIController_Reset(FOC_PIControllerTypeDef *pi);     // 重置PI控制器(积分项清零)
float FOC_PIController_Update(FOC_PIControllerTypeDef *pi,   float target, float feedback);               
void FOC_PIController_SetGains(FOC_PIControllerTypeDef *pi,   float kp,   float ki); // 设置PI比例和积分系数                  
void FOC_PIController_SetOutputLimit(FOC_PIControllerTypeDef *pi, float output_min, float output_max); // 设置PI输出限幅范围         
#endif /* __FOC_PI_CONTROLLER_H__ */
