#ifndef __FOC_CURRENT_LOOP_H__
#define __FOC_CURRENT_LOOP_H__

#include "stm32f4xx.h"
#include "foc_math.h"
#include "foc_pi_controller.h"

#define FOC_CURRENT_LOOP_DEFAULT_ID_REF          0.0f
#define FOC_CURRENT_LOOP_DEFAULT_IQ_REF          0.5f
#define FOC_CURRENT_LOOP_DEFAULT_KP              0.5f
#define FOC_CURRENT_LOOP_DEFAULT_KI              20.0f
#define FOC_CURRENT_LOOP_DEFAULT_CONTROL_PERIOD  0.001f
#define FOC_CURRENT_LOOP_DEFAULT_OUTPUT_LIMIT    3.0f

typedef struct
{
    float IdRef;
    float IqRef;
    float IdFeedback;
    float IqFeedback;
    float UdOutput;
    float UqOutput;
} FOC_CurrentLoopStateTypeDef;

void FOC_CurrentLoop_Init(void);
void FOC_CurrentLoop_Reset(void);
void FOC_CurrentLoop_SetTarget(float id_ref, float iq_ref);
void FOC_CurrentLoop_SetPI(float kp, float ki);
void FOC_CurrentLoop_SetControlPeriod(float control_period);
void FOC_CurrentLoop_SetOutputLimit(float output_limit);
FOC_DQTypeDef FOC_CurrentLoop_Update(FOC_DQTypeDef current_dq);
FOC_CurrentLoopStateTypeDef FOC_CurrentLoop_GetState(void);

#endif /* __FOC_CURRENT_LOOP_H__ */
