#include "foc_open_loop.h"
#include "foc_math.h"
#include "bsp_tim8_pwm.h"

static OpenLoopFOC_StateTypeDef open_loop_foc_state;

//°²È«Êä³ö
static void OpenLoopFOC_ApplySafeOutput(void)
{
    TIM8_PWM_SetDuty(0u, 0u, 0u);
}

void OpenLoopFOC_Init(void)
{
    open_loop_foc_state.ThetaE = 0.0f;
    open_loop_foc_state.UqRef = 0.0f;
    open_loop_foc_state.UdRef = OPEN_LOOP_FOC_DEFAULT_UD;
    open_loop_foc_state.SpeedRefRadS = 0.0f;
    open_loop_foc_state.BusVoltage = OPEN_LOOP_FOC_DEFAULT_BUS_VOLTAGE;
    open_loop_foc_state.ControlPeriodSec = OPEN_LOOP_FOC_DEFAULT_CONTROL_PERIOD;
    open_loop_foc_state.IsRunning = OPEN_LOOP_FOC_DEFAULT_RUNING;

    OpenLoopFOC_ApplySafeOutput();
}

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

void OpenLoopFOC_Stop(void)
{
    open_loop_foc_state.IsRunning = 0u;
    open_loop_foc_state.UqRef = 0.0f;

    OpenLoopFOC_ApplySafeOutput();
    TIM8_PWM_Stop();
}

void OpenLoopFOC_SetTarget(float uq_ref, float speed_ref_rad_s)
{
    uq_ref = FOC_Clamp(uq_ref, OPEN_LOOP_FOC_UQ_MIN, OPEN_LOOP_FOC_UQ_MAX);
    speed_ref_rad_s = FOC_Clamp(speed_ref_rad_s, OPEN_LOOP_FOC_SPEED_MIN_RAD_S, OPEN_LOOP_FOC_SPEED_MAX_RAD_S);

    open_loop_foc_state.UqRef = uq_ref;
    open_loop_foc_state.SpeedRefRadS = speed_ref_rad_s;
}

void OpenLoopFOC_SetBusVoltage(float bus_voltage)
{
    if ((bus_voltage >= OPEN_LOOP_FOC_BUS_VOLTAGE_MIN) && (bus_voltage <= OPEN_LOOP_FOC_BUS_VOLTAGE_MAX))
    {
        open_loop_foc_state.BusVoltage = bus_voltage;
    }
}

void OpenLoopFOC_SetControlPeriod(float control_period_sec)
{
    if ((control_period_sec >= OPEN_LOOP_FOC_PERIOD_MIN_SEC) && (control_period_sec <= OPEN_LOOP_FOC_PERIOD_MAX_SEC))
    {
        open_loop_foc_state.ControlPeriodSec = control_period_sec;
    }
}

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
    duty = FOC_SVPWM(voltage_alpha_beta.Alpha,
                     voltage_alpha_beta.Beta,
                     open_loop_foc_state.BusVoltage,
                     TIM8_PWM_GetPeriod());

    TIM8_PWM_SetDuty(duty.DutyA, duty.DutyB, duty.DutyC);
}

uint8_t OpenLoopFOC_GetIsRunning(void)
{
    return open_loop_foc_state.IsRunning;
}

OpenLoopFOC_StateTypeDef OpenLoopFOC_GetState(void)
{
    return open_loop_foc_state;
}
