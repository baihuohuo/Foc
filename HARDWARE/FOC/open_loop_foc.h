#ifndef __OPEN_LOOP_FOC_H
#define __OPEN_LOOP_FOC_H

#include "stm32f4xx.h"

/* -----------------------宏定义 -------------------*/
#define OPEN_LOOP_FOC_DEFAULT_BUS_VOLTAGE      24.0f       /* 默认母线电压, 单位: V */
#define OPEN_LOOP_FOC_DEFAULT_CONTROL_PERIOD   0.001f      /* 默认控制周期, 单位: s */
#define OPEN_LOOP_FOC_DEFAULT_UQ               1.0f        /* 默认Q轴电压, 单位: V */
#define OPEN_LOOP_FOC_DEFAULT_SPEED_RAD_S      6.2831853f  /* 默认电角速度, 单位: rad/s */
#define OPEN_LOOP_FOC_UQ_MIN                   0.0f        /* Q轴电压下限, 单位: V */
#define OPEN_LOOP_FOC_UQ_MAX                   3.0f        /* Q轴电压上限, 单位: V */
#define OPEN_LOOP_FOC_SPEED_MIN_RAD_S         -62.831853f  /* 电角速度下限, 单位: rad/s */
#define OPEN_LOOP_FOC_SPEED_MAX_RAD_S          62.831853f  /* 电角速度上限, 单位: rad/s */
#define OPEN_LOOP_FOC_BUS_VOLTAGE_MIN          6.0f        /* 母线电压下限, 单位: V */
#define OPEN_LOOP_FOC_BUS_VOLTAGE_MAX          60.0f       /* 母线电压上限, 单位: V */
#define OPEN_LOOP_FOC_PERIOD_MIN_SEC           0.0001f     /* 控制周期下限, 单位: s */
#define OPEN_LOOP_FOC_PERIOD_MAX_SEC           0.01f       /* 控制周期上限, 单位: s */

/* -----------------------类型定义 -------------------*/
typedef struct
{
  float ThetaE;             // 电角度, 单位: rad
  float UqRef;              // Q轴电压参考, 单位: V
  float UdRef;              // D轴电压参考, 单位: V
  float SpeedRefRadS;       // 电角速度参考, 单位: rad/s
  float BusVoltage;         // 母线电压, 单位: V
  float ControlPeriodSec;   // 控制周期, 单位: s
  uint8_t IsRunning;         // 运行状态: 0=停止, 1=运行
}OpenLoopFOC_StateTypeDef;

/* -----------------------函数声明 -------------------*/
// 开环FOC控制函数
void OpenLoopFOC_Init(void);                                          // 初始化开环FOC状态
void OpenLoopFOC_Start(void);                                         // 启动开环FOC
void OpenLoopFOC_Stop(void);                                          // 停止开环FOC
void OpenLoopFOC_SetTarget(float uq_ref, float speed_ref_rad_s);      // 设置Q轴电压和电角速度目标
void OpenLoopFOC_SetBusVoltage(float bus_voltage);                    // 设置母线电压
void OpenLoopFOC_SetControlPeriod(float control_period_sec);          // 设置控制周期
void OpenLoopFOC_Update(void);                                        // 执行一次开环FOC更新
uint8_t OpenLoopFOC_GetIsRunning(void);                               // 获取运行状态
OpenLoopFOC_StateTypeDef OpenLoopFOC_GetState(void);                  // 获取当前开环FOC状态

#endif /* __OPEN_LOOP_FOC_H */
