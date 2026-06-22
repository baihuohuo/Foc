/* -----------------------FOC 数学库 -------------------*/
#include "foc_math.h"
#include <math.h>

// 零矢量占空比生成: 三相都输出50% (中性点)
static FOC_SVPWMDutyTypeDef FOC_ZeroDuty(uint16_t pwm_period)
{
    FOC_SVPWMDutyTypeDef duty;
    duty.DutyA = pwm_period / 2u;
    duty.DutyB = pwm_period / 2u;
    duty.DutyC = pwm_period / 2u;
    return duty;
}

// 参数限制函数: 把value限制在[min_value, max_value]范围内
float FOC_Clamp(float value, float min_value, float max_value)
{
    if( value > max_value) return max_value;
    if( value < min_value) return min_value;
    return value;
}

// 角度归一化函数: 把角度限制在[0, 2π)范围内
float FOC_NormalizeAngle(float angle_rad)
{
    while (angle_rad >= FOC_TWO_PI)
    {
        angle_rad -= FOC_TWO_PI;
    }

    while (angle_rad < 0.0f)
    {
        angle_rad += FOC_TWO_PI;
    }

    return angle_rad;
}

// Park变换: Alpha/Beta电压 -> d/q电压
FOC_DQTypeDef FOC_Park(FOC_AlphaBetaTypeDef alpha_beta, float angle_rad)
{
    FOC_DQTypeDef voltage_dq;

    voltage_dq.D = alpha_beta.Alpha * cosf(angle_rad) + alpha_beta.Beta * sinf(angle_rad);
    voltage_dq.Q = -alpha_beta.Alpha * sinf(angle_rad) + alpha_beta.Beta * cosf(angle_rad);

    return voltage_dq;
}

// 反Park变换: d/q电压 -> Alpha/Beta电压
FOC_AlphaBetaTypeDef FOC_InversePark(FOC_DQTypeDef voltage_dq, float angle_rad)
{
    FOC_AlphaBetaTypeDef voltage_AlphaBeta;

    voltage_AlphaBeta.Alpha = voltage_dq.D * cosf(angle_rad) - voltage_dq.Q * sinf(angle_rad);
    voltage_AlphaBeta.Beta = voltage_dq.D * sinf(angle_rad) + voltage_dq.Q * cosf(angle_rad);

    return voltage_AlphaBeta;
}

// Clarke变换: A/B/C电流 -> Alpha/Beta电流
FOC_AlphaBetaTypeDef FOC_Clarke(FOC_IabcTypeDef Iabc)
{
    FOC_AlphaBetaTypeDef I_AlphaBeta;
    I_AlphaBeta.Alpha = Iabc.Ia;
    I_AlphaBeta.Beta = (Iabc.Ia + 2 * Iabc.Ib)*0.577350269f;
    
    return I_AlphaBeta;
}


// SVPWM调制: Alpha/Beta电压 -> 三相PWM比较值
FOC_SVPWMDutyTypeDef FOC_SVPWM(float u_alpha, float u_beta, float u_bus, uint16_t pwm_period)
{
    FOC_SVPWMDutyTypeDef duty;
    float angle_rad;
    float angle_in_sector_rad;
    float voltage_ref;
    float voltage_limit;
    float active_time_1;
    float active_time_2;
    float zero_time;
    float half_zero_time;
    float duty_a_ratio;
    float duty_b_ratio;
    float duty_c_ratio;
    uint8_t sector;

    // 参数检查: 无效输入时输出零矢量, 保护电机 
    if ((u_bus <= 0.0f) || (pwm_period == 0u)) return FOC_ZeroDuty(pwm_period);

    // Step 0: 初始化为中性点占空比 
    duty = FOC_ZeroDuty(pwm_period);

    // Step 1: 计算电压矢量在α/β坐标系下的电角度 
    angle_rad = atan2f(u_beta, u_alpha);
    angle_rad = FOC_NormalizeAngle(angle_rad);

    // Step 2: 将一个电周期划分为六个60°的SVPWM扇区 
    sector = (uint8_t)(angle_rad / (FOC_PI / 3.0f)) + 1;
    if (sector > 6) sector = 6;

    // Step 3: 计算当前扇区内的角度偏移, 范围: 0~60° 
    angle_in_sector_rad = angle_rad - ((float)(sector - 1u) * (FOC_PI / 3.0f));

    // Step 4: 计算参考电压矢量的幅值并限幅 (内切圆 = u_bus/√3) 
    voltage_ref = sqrtf((u_alpha * u_alpha) + (u_beta * u_beta));
    voltage_limit = u_bus * 0.577350269f;
    voltage_ref = FOC_Clamp(voltage_ref, 0.0f, voltage_limit);

    // Step 5: 计算两个有效矢量的作用时间和零矢量时间 
    active_time_1 = 1.732050808f * voltage_ref / u_bus * sinf((FOC_PI / 3.0f) - angle_in_sector_rad);
    active_time_2 = 1.732050808f * voltage_ref / u_bus * sinf(angle_in_sector_rad);
    zero_time = 1.0f - active_time_1 - active_time_2;

    if (zero_time < 0.0f)
    {
        zero_time = 0.0f;
    }

    half_zero_time = zero_time * 0.5f;

    // Step 6: 根据扇区将有效/零矢量时间转换为A/B/C三相占空比 
    switch (sector)
    {
        case 1:
            duty_a_ratio = active_time_1 + active_time_2 + half_zero_time;
            duty_b_ratio = active_time_2 + half_zero_time;
            duty_c_ratio = half_zero_time;
            break;

        case 2:
            duty_a_ratio = active_time_1 + half_zero_time;
            duty_b_ratio = active_time_1 + active_time_2 + half_zero_time;
            duty_c_ratio = half_zero_time;
            break;

        case 3:
            duty_a_ratio = half_zero_time;
            duty_b_ratio = active_time_1 + active_time_2 + half_zero_time;
            duty_c_ratio = active_time_2 + half_zero_time;
            break;

        case 4:
            duty_a_ratio = half_zero_time;
            duty_b_ratio = active_time_1 + half_zero_time;
            duty_c_ratio = active_time_1 + active_time_2 + half_zero_time;
            break;

        case 5:
            duty_a_ratio = active_time_2 + half_zero_time;
            duty_b_ratio = half_zero_time;
            duty_c_ratio = active_time_1 + active_time_2 + half_zero_time;
            break;

        case 6:
        default:
            duty_a_ratio = active_time_1 + active_time_2 + half_zero_time;
            duty_b_ratio = half_zero_time;
            duty_c_ratio = active_time_1 + half_zero_time;
            break;
    }

    // Step 7: 将占空比限制在PWM有效范围内 
    duty_a_ratio = FOC_Clamp(duty_a_ratio, 0.0f, 1.0f);
    duty_b_ratio = FOC_Clamp(duty_b_ratio, 0.0f, 1.0f);
    duty_c_ratio = FOC_Clamp(duty_c_ratio, 0.0f, 1.0f);

    //Step 8: 将占空比转换为TIM8的CCR比较寄存器值 
    duty.DutyA = (uint16_t)(duty_a_ratio * (float)pwm_period);
    duty.DutyB = (uint16_t)(duty_b_ratio * (float)pwm_period);
    duty.DutyC = (uint16_t)(duty_c_ratio * (float)pwm_period);

    return duty;
}

