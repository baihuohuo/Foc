#include "foc_pi_controller.h"

// 限幅函数: 将value限制在[min_value, max_value]范围内
static float FOC_PIController_Clamp(float value, float min_value, float max_value)
{
    if (value > max_value)
    {
        return max_value;
    }

    if (value < min_value)
    {
        return min_value;
    }

    return value;
}

// 初始化PI控制器
void FOC_PIController_Init(FOC_PIControllerTypeDef *pi,
                           float kp,
                           float ki,
                           float control_period,
                           float output_min,
                           float output_max)
{
    if (pi == 0)
    {
        return;
    }

    pi->Kp = kp;
    pi->Ki = ki;
    pi->ControlPeriod = control_period;
    pi->OutputMin = output_min;
    pi->OutputMax = output_max;
    pi->ErrorLast = 0.0f;
    pi->OutputLast = 0.0f;
}

// 重置PI控制器, 将积分项清零
void FOC_PIController_Reset(FOC_PIControllerTypeDef *pi)
{
    if (pi == 0)
    {
        return;
    }

    pi->ErrorLast = 0.0f;
    pi->OutputLast = 0.0f;
}

// PI控制器更新计算(增量式)
float FOC_PIController_Update(FOC_PIControllerTypeDef *pi, float target, float feedback)
{
    float error_now;       // 当前误差
    float output_delta;    // 输出增量
    float output_now;      // 本次输出

    if (pi == 0)
    {
        return 0.0f;
    }

    // 计算当前误差: 目标值 - 反馈值
    error_now = target - feedback;

    // 计算输出增量
    // 增量式PI: Δu = Kp * (e[k] - e[k-1]) + Ki * e[k] * T
    output_delta = pi->Kp * (error_now - pi->ErrorLast) +
                   pi->Ki * error_now * pi->ControlPeriod;

    // 累加到上次输出
    output_now = pi->OutputLast + output_delta;

    // 输出限幅
    output_now = FOC_PIController_Clamp(output_now, pi->OutputMin, pi->OutputMax);

    // 保存本次误差和输出, 供下次调用使用
    pi->ErrorLast = error_now;
    pi->OutputLast = output_now;

    return output_now;
}

// 设置PI比例和积分系数
void FOC_PIController_SetGains(FOC_PIControllerTypeDef *pi, float kp, float ki)
{
    if (pi == 0)
    {
        return;
    }

    pi->Kp = kp;
    pi->Ki = ki;
}

// 设置PI输出限幅范围
void FOC_PIController_SetOutputLimit(FOC_PIControllerTypeDef *pi, float output_min, float output_max)
{
    if (pi == 0)
    {
        return;
    }

    pi->OutputMin = output_min;
    pi->OutputMax = output_max;

    // 如果当前输出已经超出新范围, 立即限幅
    pi->OutputLast = FOC_PIController_Clamp(pi->OutputLast, pi->OutputMin, pi->OutputMax);
}
