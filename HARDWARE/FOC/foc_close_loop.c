#include "foc_close_loop.h"

static FOC_CloseLoopStateTypeDef foc_close_loop_state;
static FOC_PIControllerTypeDef foc_close_loop_d_pi;
static FOC_PIControllerTypeDef foc_close_loop_q_pi;

void FOC_CloseLoop_Init(void) 
{
    foc_close_loop_state.IdRef = FOC_CLOSE_LOOP_DEFAULT_ID_REF;
    foc_close_loop_state.IqRef = FOC_CLOSE_LOOP_DEFAULT_IQ_REF;
    foc_close_loop_state.IdFeedback = 0.0f;
    foc_close_loop_state.IqFeedback = 0.0f;
    foc_close_loop_state.UdOutput = 0.0f;
    foc_close_loop_state.UqOutput = 0.0f;

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
}

void FOC_CloseLoop_Reset(void)
{
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

void FOC_CloseLoop_SetControlPeriod(float control_period)
{
    foc_close_loop_d_pi.ControlPeriod = control_period;
    foc_close_loop_q_pi.ControlPeriod = control_period;
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

//通过两个 PI 控制器算出应该施加的 Ud/Uq 电压
FOC_DQTypeDef FOC_CloseLoop_Update(FOC_DQTypeDef feedback_dq)
{
    FOC_DQTypeDef voltage_dq;

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

    return voltage_dq;
}

FOC_CloseLoopStateTypeDef FOC_CloseLoop_GetState(void)
{
    return foc_close_loop_state;
}
