#ifndef __OPEN_LOOP_FOC_H
#define __OPEN_LOOP_FOC_H

#include "stm32f4xx.h"

#define OPEN_LOOP_FOC_DEFAULT_SPEED_RAD_S      6.2831853f   /* 默认电角速度, 单位: rad/s, 约等于 1 电周期/s */
#define OPEN_LOOP_FOC_DEFAULT_UQ               1.0f         /* 默认 q 轴电压, 单位: V, 决定开环输出力矩 */
#define OPEN_LOOP_FOC_UQ_MIN                   0.0f         /* q 轴电压下限, 单位: V */
#define OPEN_LOOP_FOC_UQ_MAX                   3.0f         /* q 轴电压上限, 单位: V */

#define OPEN_LOOP_FOC_DEFAULT_UD               0.0f         /* 默认 d 轴电压, 单位: V, 开环一般设为 0 */

#define OPEN_LOOP_FOC_SPEED_MIN_RAD_S         -62.831853f   /* 最小电角速度, 单位: rad/s, 约 -10 电周期/s */
#define OPEN_LOOP_FOC_SPEED_MAX_RAD_S          62.831853f   /* 最大电角速度, 单位: rad/s, 约 10 电周期/s */

#define OPEN_LOOP_FOC_DEFAULT_BUS_VOLTAGE      24.0f        /* 默认直流母线电压, 单位: V */
#define OPEN_LOOP_FOC_BUS_VOLTAGE_MIN          6.0f         /* 母线电压下限, 单位: V */
#define OPEN_LOOP_FOC_BUS_VOLTAGE_MAX          60.0f        /* 母线电压上限, 单位: V */

#define OPEN_LOOP_FOC_DEFAULT_CONTROL_PERIOD   0.001f       /* 默认控制周期, 单位: s, 这里是 1ms */
#define OPEN_LOOP_FOC_PERIOD_MIN_SEC           0.0001f      /* 控制周期下限, 单位: s */
#define OPEN_LOOP_FOC_PERIOD_MAX_SEC           0.01f        /* 控制周期上限, 单位: s */

#define OPEN_LOOP_FOC_DEFAULT_RUNING           0u           /* 默认运行状态: 0=停止, 1=运行 */

typedef struct
{
    float ThetaE;               /* 默认角度 */
    float UqRef;                /* q 轴电压 */
    float UdRef;                /* d 轴电压 */
    float SpeedRefRadS;         /* 默认电角速度 */
    float BusVoltage;           /* 直流母线电压 */
    float ControlPeriodSec;     /* 控制周期 */
    uint8_t IsRunning;          /* 运行状态 */
} OpenLoopFOC_StateTypeDef;

void OpenLoopFOC_Init(void);
void OpenLoopFOC_Start(void);
void OpenLoopFOC_Stop(void);
void OpenLoopFOC_SetTarget(float uq_ref, float speed_ref_rad_s);
void OpenLoopFOC_SetBusVoltage(float bus_voltage);
void OpenLoopFOC_SetControlPeriod(float control_period_sec);
void OpenLoopFOC_Update(void);
uint8_t OpenLoopFOC_GetIsRunning(void);
OpenLoopFOC_StateTypeDef OpenLoopFOC_GetState(void);

#endif /* __OPEN_LOOP_FOC_H */
