#ifndef __FOC_MATH_H
#define __FOC_MATH_H

#include "stm32f4xx.h"

/* -----------------------宏定义 -------------------*/
#define FOC_PI                         3.14159265358979323846f  /* 圆周率, 单位: rad */
#define FOC_TWO_PI                     6.28318530717958647692f  /* 2*pi, 一个电周期 */


/* -----------------------类型定义 -------------------*/
typedef struct
{
  float Alpha;  // Alpha轴电压  
  float Beta;   // Beta轴电压 
}FOC_AlphaBetaTypeDef;

typedef struct
{
  float D;  // D轴电压  
  float Q;  // Q轴电压  
}FOC_DQTypeDef;

typedef struct
{
    float Ia;
    float Ib;
    float Ic;
} FOC_IabcTypeDef;

typedef struct
{
  uint16_t DutyA;  // A相PWM 
  uint16_t DutyB;  // B相PWM 
  uint16_t DutyC;  // C相PWM 
}FOC_SVPWMDutyTypeDef;

/* -----------------------函数声明 -------------------*/

// 参数限制函数
float FOC_Clamp(float value, float min_value, float max_value);
float FOC_NormalizeAngle(float angle);

//电机函数
FOC_DQTypeDef FOC_Park(FOC_AlphaBetaTypeDef  alpha_beta, float angle_rad); // Park变换: Alpha/Beta电压 -> d/q电压
FOC_AlphaBetaTypeDef FOC_InversePark(FOC_DQTypeDef voltage_dq, float angle_rad); // 反Park变换: d/q电压 -> Alpha/Beta电压
FOC_AlphaBetaTypeDef FOC_Clarke(FOC_IabcTypeDef Iabc); // CLARK变换: A/B/C电流 -> d/q电流
FOC_SVPWMDutyTypeDef FOC_SVPWM(float u_alpha, float u_beta, float u_bus, uint16_t pwm_period);//SVPWM: Alpha/Beta电压 -> 三相PWM比较值 

#endif /* __FOC_MATH_H */
