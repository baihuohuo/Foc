/* -----------------------开环 FOC 控制 -------------------*/
#include "open_loop_foc.h"
#include "foc_math.h"
#include "bsp_tim8_pwm.h"

// 开环FOC全局状态 (文件内私有, 仅通过GetState对外暴露)
static OpenLoopFOC_StateTypeDef open_loop_foc_state;

// 安全输出: 关闭力矩输出并清零三相比较值
static void OpenLoopFOC_ApplySafeOutput(void)
{
    TIM8_PWM_SetDuty(0u, 0u, 0u);
}

// 初始化开环FOC状态: 加载默认电压、速度、控制周期
void OpenLoopFOC_Init(void)
{
    open_loop_foc_state.ThetaE = 0.0f;
    open_loop_foc_state.UqRef = 0.0f;
    open_loop_foc_state.UdRef = 0.0f;
    open_loop_foc_state.SpeedRefRadS = 0.0f;
    open_loop_foc_state.BusVoltage = OPEN_LOOP_FOC_DEFAULT_BUS_VOLTAGE;
    open_loop_foc_state.ControlPeriodSec = OPEN_LOOP_FOC_DEFAULT_CONTROL_PERIOD;
    open_loop_foc_state.IsRunning = 0u;
    OpenLoopFOC_ApplySafeOutput();
}

// 启动开环FOC
void OpenLoopFOC_Start(void)
{
    if (open_loop_foc_state.UqRef <= 0.0f)
    {
        open_loop_foc_state.UqRef = OPEN_LOOP_FOC_DEFAULT_UQ;
    }

    if (open_loop_foc_state.SpeedRefRadS == 0.0f)
    {
        open_loop_foc_state.SpeedRefRadS = OPEN_LOOP_FOC_DEFAULT_SPEED_RAD_S;
    }

    open_loop_foc_state.IsRunning = 1u;
    TIM8_PWM_Start();
}

// 停止开环FOC
void OpenLoopFOC_Stop(void)
{
    open_loop_foc_state.IsRunning = 0u;
    open_loop_foc_state.UqRef = 0.0f;
    OpenLoopFOC_ApplySafeOutput();
    TIM8_PWM_Stop();
}

// 设置Q轴电压和电角速度目标
void OpenLoopFOC_SetTarget(float uq_ref, float speed_ref_rad_s)
{
    uq_ref = FOC_Clamp(uq_ref, OPEN_LOOP_FOC_UQ_MIN, OPEN_LOOP_FOC_UQ_MAX);
    speed_ref_rad_s = FOC_Clamp(speed_ref_rad_s, OPEN_LOOP_FOC_SPEED_MIN_RAD_S, OPEN_LOOP_FOC_SPEED_MAX_RAD_S);

    open_loop_foc_state.UqRef = uq_ref;
    open_loop_foc_state.SpeedRefRadS = speed_ref_rad_s;
}

// 设置母线电压
void OpenLoopFOC_SetBusVoltage(float bus_voltage)
{
    if ((bus_voltage >= OPEN_LOOP_FOC_BUS_VOLTAGE_MIN) && (bus_voltage <= OPEN_LOOP_FOC_BUS_VOLTAGE_MAX))
    {
        open_loop_foc_state.BusVoltage = bus_voltage;
    }
}

// 设置控制周期 (秒)
void OpenLoopFOC_SetControlPeriod(float control_period_sec)
{
    if ((control_period_sec >= OPEN_LOOP_FOC_PERIOD_MIN_SEC) && (control_period_sec <= OPEN_LOOP_FOC_PERIOD_MAX_SEC))
    {
        open_loop_foc_state.ControlPeriodSec = control_period_sec;
    }
}

// 执行一次开环FOC更新: 电角度按速度积分并归一化
void OpenLoopFOC_Update(void)
{
    FOC_DQTypeDef voltage_dq;
    FOC_AlphaBetaTypeDef voltage_alpha_beta;
    FOC_SVPWMDutyTypeDef duty;

    if (open_loop_foc_state.IsRunning == 0u)
    {
        OpenLoopFOC_ApplySafeOutput();
        return;
    }

    open_loop_foc_state.ThetaE += open_loop_foc_state.SpeedRefRadS * open_loop_foc_state.ControlPeriodSec;
    open_loop_foc_state.ThetaE = FOC_NormalizeAngle(open_loop_foc_state.ThetaE);

    voltage_dq.D = open_loop_foc_state.UdRef;
    voltage_dq.Q = open_loop_foc_state.UqRef;

    voltage_alpha_beta = FOC_InversePark(voltage_dq, open_loop_foc_state.ThetaE);

    duty = FOC_SVPWM(voltage_alpha_beta.Alpha, voltage_alpha_beta.Beta, open_loop_foc_state.BusVoltage, TIM8_PWM_GetPeriod());

    TIM8_PWM_SetDuty(duty.DutyA, duty.DutyB, duty.DutyC);
}

// 获取运行状态
uint8_t OpenLoopFOC_GetIsRunning(void)
{
    return open_loop_foc_state.IsRunning;
}

// 获取当前开环FOC状态 (返回副本, 避免外部直接修改)
OpenLoopFOC_StateTypeDef OpenLoopFOC_GetState(void)
{
    return open_loop_foc_state;
}
