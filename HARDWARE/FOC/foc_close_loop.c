#include "foc_close_loop.h"
#include "bsp_adc.h"
#include "bsp_tim8_pwm.h"

static FOC_CloseLoopStateTypeDef foc_close_loop_state;
static FOC_PIControllerTypeDef foc_close_loop_d_pi;
static FOC_PIControllerTypeDef foc_close_loop_q_pi;

static void FOC_CloseLoop_ApplySafeOutput(void)
{
    TIM8_PWM_SetDuty(0u, 0u, 0u);
}

void FOC_CloseLoop_Init(void)
{
    foc_close_loop_state.ThetaE = 0.0f;

    foc_close_loop_state.IdRef = FOC_CLOSE_LOOP_DEFAULT_ID_REF;
    foc_close_loop_state.IqRef = FOC_CLOSE_LOOP_DEFAULT_IQ_REF;
    foc_close_loop_state.IdFeedback = 0.0f;
    foc_close_loop_state.IqFeedback = 0.0f;

    foc_close_loop_state.UdOutput = 0.0f;
    foc_close_loop_state.UqOutput = 0.0f;

    foc_close_loop_state.SpeedRefRadS = FOC_CLOSE_LOOP_DEFAULT_SPEED_RAD_S;
    foc_close_loop_state.BusVoltage = FOC_CLOSE_LOOP_DEFAULT_BUS_VOLTAGE;
    foc_close_loop_state.ControlPeriodSec = FOC_CLOSE_LOOP_DEFAULT_CONTROL_PERIOD;
    foc_close_loop_state.IsRunning = 0u;

    FOC_PIController_Init(&foc_close_loop_d_pi,
                          FOC_CLOSE_LOOP_DEFAULT_KP,
                          FOC_CLOSE_LOOP_DEFAULT_KI,
                          FOC_CLOSE_LOOP_DEFAULT_CONTROL_PERIOD,
                          -FOC_CLOSE_LOOP_DEFAULT_OUTPUT_LIMIT,
                          FOC_CLOSE_LOOP_DEFAULT_OUTPUT_LIMIT);

    FOC_PIController_Init(&foc_close_loop_q_pi,
                          FOC_CLOSE_LOOP_DEFAULT_KP,
                          FOC_CLOSE_LOOP_DEFAULT_KI,
                          FOC_CLOSE_LOOP_DEFAULT_CONTROL_PERIOD,
                          -FOC_CLOSE_LOOP_DEFAULT_OUTPUT_LIMIT,
                          FOC_CLOSE_LOOP_DEFAULT_OUTPUT_LIMIT);

    FOC_CloseLoop_ApplySafeOutput();
}

void FOC_CloseLoop_Start(void)
{
    FOC_CloseLoop_Reset();
    foc_close_loop_state.IsRunning = 1u;
    TIM8_PWM_Start();
}

void FOC_CloseLoop_Stop(void)
{
    foc_close_loop_state.IsRunning = 0u;
    FOC_CloseLoop_ApplySafeOutput();
    TIM8_PWM_Stop();
}

void FOC_CloseLoop_Reset(void)
{
	foc_close_loop_state.ThetaE = 0.0f;
    foc_close_loop_state.IdFeedback = 0.0f;
    foc_close_loop_state.IqFeedback = 0.0f;
    foc_close_loop_state.UdOutput = 0.0f;
    foc_close_loop_state.UqOutput = 0.0f;

    FOC_PIController_Reset(&foc_close_loop_d_pi);
    FOC_PIController_Reset(&foc_close_loop_q_pi);
}

void FOC_CloseLoop_SetTarget(float id_ref, float iq_ref)
{
    foc_close_loop_state.IdRef = id_ref;
    foc_close_loop_state.IqRef = iq_ref;
}

void FOC_CloseLoop_SetPI(float kp, float ki)
{
    FOC_PIController_SetGains(&foc_close_loop_d_pi, kp, ki);
    FOC_PIController_SetGains(&foc_close_loop_q_pi, kp, ki);
}

void FOC_CloseLoop_SetSpeed(float speed_ref_rad_s)
{
    foc_close_loop_state.SpeedRefRadS = speed_ref_rad_s;
}

void FOC_CloseLoop_SetBusVoltage(float bus_voltage)
{
    if (bus_voltage > 0.0f)
    {
        foc_close_loop_state.BusVoltage = bus_voltage;
    }
}

void FOC_CloseLoop_SetControlPeriod(float control_period)
{
    if (control_period > 0.0f)
    {
        foc_close_loop_state.ControlPeriodSec = control_period;
        foc_close_loop_d_pi.ControlPeriod = control_period;
        foc_close_loop_q_pi.ControlPeriod = control_period;
    }
}

void FOC_CloseLoop_SetOutputLimit(float output_limit)
{
    if (output_limit < 0.0f)
    {
        output_limit = -output_limit;
    }

    FOC_PIController_SetOutputLimit(&foc_close_loop_d_pi, -output_limit, output_limit);
    FOC_PIController_SetOutputLimit(&foc_close_loop_q_pi, -output_limit, output_limit);
}

void FOC_CloseLoop_Update(void)
{
    BSP_ADC_DataTypeDef adc_data;
    FOC_IabcTypeDef current_abc;
    FOC_AlphaBetaTypeDef current_alpha_beta;
    FOC_DQTypeDef feedback_dq;
    FOC_DQTypeDef voltage_dq;
    FOC_AlphaBetaTypeDef voltage_alpha_beta;
    FOC_SVPWMDutyTypeDef duty;

    if (foc_close_loop_state.IsRunning == 0u)
    {
        FOC_CloseLoop_ApplySafeOutput();
        return;
    }

    foc_close_loop_state.ThetaE = BSP_HALL_GetElectricalAngle();

    BSP_ADC_Update();
    adc_data = BSP_ADC_GetAllData();

    current_abc.Ia = adc_data.RealPhaseMT2_AMPU;
    current_abc.Ib = adc_data.RealPhaseMT2_AMPV;
    current_abc.Ic = adc_data.RealPhaseMT2_AMPW;

    current_alpha_beta = FOC_Clarke(current_abc);
    feedback_dq = FOC_Park(current_alpha_beta, foc_close_loop_state.ThetaE);

    foc_close_loop_state.IdFeedback = feedback_dq.D;
    foc_close_loop_state.IqFeedback = feedback_dq.Q;

    voltage_dq.D = FOC_PIController_Update(&foc_close_loop_d_pi,
                                           foc_close_loop_state.IdRef,
                                           foc_close_loop_state.IdFeedback);

    voltage_dq.Q = FOC_PIController_Update(&foc_close_loop_q_pi,
                                           foc_close_loop_state.IqRef,
                                           foc_close_loop_state.IqFeedback);

    foc_close_loop_state.UdOutput = voltage_dq.D;
    foc_close_loop_state.UqOutput = voltage_dq.Q;

    voltage_alpha_beta = FOC_InversePark(voltage_dq, foc_close_loop_state.ThetaE);

    duty = FOC_SVPWM(voltage_alpha_beta.Alpha,
                     voltage_alpha_beta.Beta,
                     foc_close_loop_state.BusVoltage,
                     TIM8_PWM_GetPeriod());

    TIM8_PWM_SetDuty(duty.DutyA, duty.DutyB, duty.DutyC);
}

uint8_t FOC_CloseLoop_GetIsRunning(void)
{
    return foc_close_loop_state.IsRunning;
}

FOC_CloseLoopStateTypeDef FOC_CloseLoop_GetState(void)
{
    return foc_close_loop_state;
}