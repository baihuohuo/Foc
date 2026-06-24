#ifndef __FOC_CLOSE_LOOP_H
#define __FOC_CLOSE_LOOP_H

#include "stm32f4xx.h"
#include "foc_math.h"
#include "foc_pi_controller.h"

#define FOC_CLOSE_LOOP_DEFAULT_ID_REF          0.0f
#define FOC_CLOSE_LOOP_DEFAULT_IQ_REF          0.3f
#define FOC_CLOSE_LOOP_DEFAULT_KP              0.05f
#define FOC_CLOSE_LOOP_DEFAULT_KI              5.0f
#define FOC_CLOSE_LOOP_DEFAULT_CONTROL_PERIOD  0.001f
#define FOC_CLOSE_LOOP_DEFAULT_OUTPUT_LIMIT    1.0f
#define FOC_CLOSE_LOOP_DEFAULT_SPEED_RAD_S     6.2831853f
#define FOC_CLOSE_LOOP_DEFAULT_BUS_VOLTAGE     24.0f

typedef struct
{
    float ThetaE;

    float IdRef;
    float IqRef;
    float IdFeedback;
    float IqFeedback;

    float UdOutput;
    float UqOutput;

    float SpeedRefRadS;
    float BusVoltage;
    float ControlPeriodSec;

    uint8_t IsRunning;
} FOC_CloseLoopStateTypeDef;

void FOC_CloseLoop_Init(void);
void FOC_CloseLoop_Start(void);
void FOC_CloseLoop_Stop(void);
void FOC_CloseLoop_Reset(void);
void FOC_CloseLoop_SetTarget(float id_ref, float iq_ref);
void FOC_CloseLoop_SetPI(float kp, float ki);
void FOC_CloseLoop_SetSpeed(float speed_ref_rad_s);
void FOC_CloseLoop_SetBusVoltage(float bus_voltage);
void FOC_CloseLoop_SetControlPeriod(float control_period);
void FOC_CloseLoop_SetOutputLimit(float output_limit);
void FOC_CloseLoop_Update(void);
uint8_t FOC_CloseLoop_GetIsRunning(void);
FOC_CloseLoopStateTypeDef FOC_CloseLoop_GetState(void);

#endif