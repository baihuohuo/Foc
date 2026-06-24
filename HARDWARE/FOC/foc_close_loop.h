#ifndef __FOC_CLOSE_LOOP_H
#define __FOC_CLOSE_LOOP_H

#include "stm32f4xx.h"
#include "foc_math.h"
#include "foc_pi_controller.h"

#define FOC_CLOSE_LOOP_DEFAULT_ID_REF          0.0f    /* 默认 d 轴电流目标, 单位: A, 一般设为 0 */
#define FOC_CLOSE_LOOP_DEFAULT_IQ_REF          0.5f    /* 默认 q 轴电流目标, 单位: A, 决定输出力矩 */
#define FOC_CLOSE_LOOP_DEFAULT_KP              0.5f    /* 默认电流环比例系数 */
#define FOC_CLOSE_LOOP_DEFAULT_KI              20.0f   /* 默认电流环积分系数 */
#define FOC_CLOSE_LOOP_DEFAULT_CONTROL_PERIOD  0.001f  /* 默认电流环控制周期, 单位: s, 这里是 1ms */
#define FOC_CLOSE_LOOP_DEFAULT_OUTPUT_LIMIT    3.0f    /* 默认电流环输出限幅, 单位: V, 限制 Ud/Uq */

typedef struct
{
    float IdRef;         /* d 轴电流目标值 */
    float IqRef;         /* q 轴电流目标值 */
    float IdFeedback;    /* d 轴电流反馈值 */
    float IqFeedback;    /* q 轴电流反馈值 */
    float UdOutput;      /* d 轴电压输出值 */
    float UqOutput;      /* q 轴电压输出值 */
} FOC_CloseLoopStateTypeDef;

void FOC_CloseLoop_Init(void);                                      /* 初始化电流闭环 */
void FOC_CloseLoop_Reset(void);                                     /* 复位电流闭环状态和 PI 积分 */
void FOC_CloseLoop_SetTarget(float id_ref, float iq_ref);           /* 设置 d/q 轴电流目标 */
void FOC_CloseLoop_SetPI(float kp, float ki);                       /* 设置电流环 PI 参数 */
void FOC_CloseLoop_SetControlPeriod(float control_period);          /* 设置电流环控制周期 */
void FOC_CloseLoop_SetOutputLimit(float output_limit);              /* 设置电流环输出限幅 */
FOC_DQTypeDef FOC_CloseLoop_Update(FOC_DQTypeDef current_dq);       /* 更新电流闭环, 输入 Id/Iq 反馈, 输出 Ud/Uq */
FOC_CloseLoopStateTypeDef FOC_CloseLoop_GetState(void);             /* 获取电流闭环状态 */

#endif /* __FOC_CLOSE_LOOP_H */
