#include "bsp_foc_close_loop.h"

static FOC_CurrentLoopStateTypeDef foc_current_loop_state;
static FOC_PIControllerTypeDef foc_current_loop_d_pi;
static FOC_PIControllerTypeDef foc_current_loop_q_pi;

void FOC_CurrentLoop_Init(void)
{
    foc_current_loop_state.IdRef = FOC_CURRENT_LOOP_DEFAULT_ID_REF;
    foc_current_loop_state.IqRef = FOC_CURRENT_LOOP_DEFAULT_IQ_REF;
    foc_current_loop_state.IdFeedback = 0.0f;
    foc_current_loop_state.IqFeedback = 0.0f;
    foc_current_loop_state.UdOutput = 0.0f;
    foc_current_loop_state.UqOutput = 0.0f;

    FOC_PIController_Init(&foc_current_loop_d_pi,
                          FOC_CURRENT_LOOP_DEFAULT_KP,
                          FOC_CURRENT_LOOP_DEFAULT_KI,
                          FOC_CURRENT_LOOP_DEFAULT_CONTROL_PERIOD,
                          -FOC_CURRENT_LOOP_DEFAULT_OUTPUT_LIMIT,
                          FOC_CURRENT_LOOP_DEFAULT_OUTPUT_LIMIT);

    FOC_PIController_Init(&foc_current_loop_q_pi,
                          FOC_CURRENT_LOOP_DEFAULT_KP,
                          FOC_CURRENT_LOOP_DEFAULT_KI,
                          FOC_CURRENT_LOOP_DEFAULT_CONTROL_PERIOD,
                          -FOC_CURRENT_LOOP_DEFAULT_OUTPUT_LIMIT,
                          FOC_CURRENT_LOOP_DEFAULT_OUTPUT_LIMIT);
}

void FOC_CurrentLoop_Reset(void)
{
    foc_current_loop_state.IdFeedback = 0.0f;
    foc_current_loop_state.IqFeedback = 0.0f;
    foc_current_loop_state.UdOutput = 0.0f;
    foc_current_loop_state.UqOutput = 0.0f;

    FOC_PIController_Reset(&foc_current_loop_d_pi);
    FOC_PIController_Reset(&foc_current_loop_q_pi);
}

void FOC_CurrentLoop_SetTarget(float id_ref, float iq_ref)
{
    foc_current_loop_state.IdRef = id_ref;
    foc_current_loop_state.IqRef = iq_ref;
}

void FOC_CurrentLoop_SetPI(float kp, float ki)
{
    FOC_PIController_SetGains(&foc_current_loop_d_pi, kp, ki);
    FOC_PIController_SetGains(&foc_current_loop_q_pi, kp, ki);
}

void FOC_CurrentLoop_SetControlPeriod(float control_period)
{
    foc_current_loop_d_pi.ControlPeriod = control_period;
    foc_current_loop_q_pi.ControlPeriod = control_period;
}

void FOC_CurrentLoop_SetOutputLimit(float output_limit)
{
    if (output_limit < 0.0f)
    {
        output_limit = -output_limit;
    }

    FOC_PIController_SetOutputLimit(&foc_current_loop_d_pi, -output_limit, output_limit);
    FOC_PIController_SetOutputLimit(&foc_current_loop_q_pi, -output_limit, output_limit);
}

FOC_DQTypeDef FOC_CurrentLoop_Update(FOC_DQTypeDef current_dq)
{
    FOC_DQTypeDef voltage_dq;

    foc_current_loop_state.IdFeedback = current_dq.D;
    foc_current_loop_state.IqFeedback = current_dq.Q;

    voltage_dq.D = FOC_PIController_Update(&foc_current_loop_d_pi,
                                           foc_current_loop_state.IdRef,
                                           foc_current_loop_state.IdFeedback);

    voltage_dq.Q = FOC_PIController_Update(&foc_current_loop_q_pi,
                                           foc_current_loop_state.IqRef,
                                           foc_current_loop_state.IqFeedback);

    foc_current_loop_state.UdOutput = voltage_dq.D;
    foc_current_loop_state.UqOutput = voltage_dq.Q;

    return voltage_dq;
}

FOC_CurrentLoopStateTypeDef FOC_CurrentLoop_GetState(void)
{
    return foc_current_loop_state;
}
